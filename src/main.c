// #include "../../freetype/freetype/include/ft2build.h"
// #include "../../freetype/freetype/include/freetype/freetype.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../include/vox.h"			/* Main project header */
#include "../include/world.h"
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"
#include "../include/block.h"
#include "../include/cube.h"
#include "../include/chunks.h"

struct s_font_context {
	GLuint 			VAO, VBO;
	GLuint 			fontShaderID;
	CharacterFont	*font;
};

struct s_character_font {
	GLuint	TextureID;	/* Glyph ID */
	vec2 	Size;		/* Glyphe Size */
	vec2 	Bearing;	/* Offset from baseline to left/top of glyph */
	GLuint	Advance;	/* Offset to advance to next glyph */
};

s8 freeTypeFontLoadChar(Context *context, FT_Face face) {

	if (!(context->fontContext->font = ft_calloc(sizeof(CharacterFont), 128))) {
		ft_printf_fd(2, "ERROR::FREETYPE: Failed to allocate memory for characters\n");
		return (FALSE);
	}

	/* Disable byte-alignment restriction */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Load first 128 characters of ASCII set */
	for (u8 c = 0; c < 128; c++) {
		/* Load character glyph */
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			ft_printf_fd(2, "ERROR::FREETYPE: Failed to load Glyph\n");
			continue ;
		}
		/* Generate texture */
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		/* Set texture options */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		/* Now store character for later use */
		CharacterFont character = {
			textureId,
			{face->glyph->bitmap.width, face->glyph->bitmap.rows},
			{face->glyph->bitmap_left, face->glyph->bitmap_top},
			face->glyph->advance.x
		};
		context->fontContext->font[c] = character;
	}
	return (TRUE);
}

void freeTypeFontInit(Context *c) {
	FT_Library	ft;
	FT_Face		face;

	if (FT_Init_FreeType(&ft)) {
		ft_printf_fd(2, "ERROR::FREETYPE: Could not init FreeType Library\n");
		return ;
	}
	if (FT_New_Face(ft, "rsc/font/arial.ttf", 0, &face)) {
		ft_printf_fd(2, "ERROR::FREETYPE: Failed to load font\n");
		return ;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);

	if (!(c->fontContext = ft_calloc(sizeof(FontContext), 1))) {
		ft_printf_fd(2, "ERROR::FREETYPE: Failed to allocate memory for font context\n");
		return ;
	}
	if (!freeTypeFontLoadChar(c, face)) {
		return ;
	}
	/* Free the free type rsc */
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	/* Configure VAO/VBO for texture quads */
	glGenVertexArrays(1, &c->fontContext->VAO);
	glGenBuffers(1, &c->fontContext->VBO);
	glBindVertexArray(c->fontContext->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, c->fontContext->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* Load font shaders */
	c->fontContext->fontShaderID = load_shader(CHAR_VERTEX_SHADER, CHAR_FRAGMENT_SHADER);
}

void chunksRender(Context *c, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunksByFace(c);
    glFlush();
}

void renderChunksVBODestroy(Context *c) {
	if (mtx_trylock(&c->vboToDestroyMtx) != thrd_success) {
		renderNeedDataSet(c, TRUE);
		return ;
	}
	// mtx_lock(&c->vboToDestroyMtx);
	for (t_list *current = c->vboToDestroy; current; current = current->next) {
		glDeleteBuffers(1, (GLuint *)current->content);
	}
	ft_lstclear(&c->vboToDestroy, free);
	mtx_unlock(&c->vboToDestroyMtx);
}

void renderChunksLoadNewVBO(Context *c) {

	// /* LOCK chunk MTX */
	if (mtx_trylock(&c->threadContext->chunkMtx) != thrd_success) {
		renderNeedDataSet(c, TRUE);
		return ;
	}

	undergroundBlockcreate(c);


	c->chunkLoadedNb = hashmap_size(c->world->chunksMap);
	c->chunkToLoadInQueue = hashmap_size(c->threadContext->chunksMapToLoad);
	mtx_lock(&c->vboToCreateMtx);
	for (t_list *current = c->vboToCreate; current; current = current->next) {
		BlockPos chunkID = *(BlockPos *)current->content;
		renderChunkCreateFaceVBO(c->world->chunksMap, chunkID);
	}
	renderNeedDataSet(c, FALSE);
	/* UNLOCK chunk MTX */
	mtx_unlock(&c->threadContext->chunkMtx);
	ft_lstclear(&c->vboToCreate, free);
	mtx_unlock(&c->vboToCreateMtx);
}

void renderChunksVBOhandling(Context *c) {
	renderChunksLoadNewVBO(c);
	renderChunksVBODestroy(c);
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->isRunningMtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->isRunningMtx);
	
	renderNeedDataSet(c, FALSE);

	thrd_join(c->threadContext->supervisor, &status);
	hashmap_destroy(c->threadContext->chunksMapToLoad);
	mtx_destroy(&c->threadContext->chunkMtx);
	// free(c->threadContext->workers);
	free(c->threadContext);

	ft_printf_fd(1, PINK"\nSupervisor thread joined with status %d\n"RESET, status);


	if (c->world->undergroundBlock->udgFaceCount != 0) {
		undergroundBlockFree(c->world->undergroundBlock);
		c->world->undergroundBlock->udgFaceCount = 0;
	}
	free(c->world->undergroundBlock);

	renderChunksVBODestroy(c);
	ft_lstclear(&c->vboToCreate, free);

	mtx_destroy(&c->renderMtx);
	mtx_destroy(&c->gameMtx);
	mtx_destroy(&c->isRunningMtx);
	mtx_destroy(&c->vboToDestroyMtx);
	mtx_destroy(&c->vboToCreateMtx);
	mtx_destroy(&c->renderDataNeededMtx);


	hashmap_destroy(c->world->renderChunksMap);
	hashmap_destroy(c->world->chunksMap);
	free(c->world);
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; ++i) {
		free(c->perlin2D[i]);
	}
	free(c->perlin2D);
	for (u32 i = 0; i < PERLIN_SNAKE_HEIGHT; ++i) {
		free(c->perlinCaveNoise[i]);
	}
	free(c->perlinCaveNoise);
	for (u32 i = 0; i < FACE_VERTEX_ARRAY_SIZE; ++i) {
		glDeleteVertexArrays(1, &c->faceCube[i].VAO);
		glDeleteBuffers(1, &c->faceCube[i].VBO);
		glDeleteBuffers(1, &c->faceCube[i].EBO);
	}
	free(c->faceCube);
	free(c);
    // free(c->perlinNoise);
	glfwTerminate();
}


void updateGame(Context *c) {
	/* Input handling */
	glfwPollEvents();

	/* Update data */
	mtx_lock(&c->gameMtx);
	// mtxLockUpdateTime(c, &c->gameMtx, &c->mtxTime.start, &c->mtxTime.end, &c->mtxTime.gameMtxTime, "Game");

	handle_input(c);
	update_camera(c, c->cubeShaderID);

	mtx_unlock(&c->gameMtx);
}

void renderGame(Context *c, GLuint skyTexture) {
	/* Update render */
	renderChunksVBOhandling(c);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* Render logic */
	displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam.projection, c->cam.view);
	chunksRender(c, c->cubeShaderID);
	glfwSwapBuffers(c->win_ptr);
}


void main_loop(Context *c, GLuint skyTexture) {
    while (!glfwWindowShouldClose(c->win_ptr)) {
		updateGame(c);
		renderGame(c, skyTexture);	
    }
}

int main() {
    Context *context;

	if (!(context = contextInit())) {
		return (1);
	}

	freeTypeFontInit(context);
	
	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);
	main_loop(context, context->skyTexture);
    vox_destroy(context);
    return (0);
}


// void testUpdateLogic(Context *c, GLuint skyTexture) {
//     double lastTime = glfwGetTime();
//     double timer = lastTime;
//     double deltaTime = 0;
//     double nowTime = 0;
//     int frames = 0;
//     int updates = 0;

//     double limitFPS = 60.0;

//     while (!glfwWindowShouldClose(c->win_ptr)) {
//         nowTime = glfwGetTime();
//         deltaTime += (nowTime - lastTime) * limitFPS; // Multiply instead of divide
//         lastTime = nowTime;
//         // Input, AI, Physics, etc.
//         while (deltaTime >= 1.0) {
//             updateGame(c); // Update game logic here
//             updates++;
//             deltaTime--;
//         }
//         // Rendering
//         renderGame(c, skyTexture); // Render game here
//         frames++;

//         // Reset after one second
//         if (glfwGetTime() - timer > 1.0) {
//             timer++;
//             VOX_PROTECTED_LOG(c, RESET_LINE""PINK"FPS: %d, Updates: %d\n"RESET, frames, updates);
//             (void)frames, (void)updates;
//             updates = 0;
//             frames = 0;
//         }
//     }
// }






/* We need special logic to destroy renderChunksCacheMap cause data store is shared with renderChunksMap */
// renderChunkCacheMapDestroy(c->world->renderChunksCacheMap, c->world->renderChunksMap);
// void renderChunkCacheMapDestroy(HashMap *renderChunkCache, HashMap *renderChunks) {
// 	HashMap_it	it = hashmap_iterator(renderChunkCache);
// 	t_list		*removeNodeList = NULL, *removeDataList = NULL;
// 	BlockPos	*tmpChunkID = NULL;
// 	s8			next = TRUE;

// 	while (hashmap_next(&it)) {
// 		BlockPos chunkID = ((RenderChunks *)it.value)->chunkID;
// 		tmpChunkID = malloc(sizeof(BlockPos));
// 		ft_memcpy(tmpChunkID, &chunkID, sizeof(BlockPos));
// 		if (chunksIsRenderer(renderChunks, chunkID)) {
// 			ft_lstadd_front(&removeNodeList, ft_lstnew(tmpChunkID));
// 		} else {
// 			ft_lstadd_front(&removeDataList, ft_lstnew(tmpChunkID));
// 		}
// 	}

// 	for (t_list *current = removeNodeList; current; current = current->next) {
// 		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_NODE);
// 	}

// 	for (t_list *current = removeDataList; current; current = current->next) {
// 		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_DATA);
// 	}
// 	ft_lstclear(&removeNodeList, free);
// 	ft_lstclear(&removeDataList, free);

// 	hashmap_destroy(renderChunkCache);
// }