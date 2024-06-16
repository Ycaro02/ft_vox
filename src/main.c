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
	mat4			projection;
};

struct s_character_font {
	GLuint	TextureID;	/* Glyph ID */
	vec2 	Size;		/* Glyphe Size */
	vec2 	Bearing;	/* Offset from baseline to left/top of glyph */
	GLuint	Advance;	/* Offset to advance to next glyph */
};


void renderFontText(Context *c, const char *text, f32 x, f32 y, f32 scale, vec3 color) {
	glUseProgram(c->fontContext->fontShaderID);
	set_shader_var_vec3(c->fontContext->fontShaderID, "textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(c->fontContext->VAO);

	/* Iterate through all characters */
	for (u32 i = 0; i < ft_strlen(text); i++) {
		CharacterFont ch = c->fontContext->font[(s8)text[i]];
		f32 xpos = x + ch.Bearing[0] * scale;
		f32 ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;
		f32 w = ch.Size[0] * scale;
		f32 h = ch.Size[1] * scale;
		/* Update VBO for each character */
		f32 vertices[6][4] = {
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos + h, 1.0f, 0.0f}
		};
		/* Render glyph texture over quad */
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		/* Update content of VBO memory */
		glBindBuffer(GL_ARRAY_BUFFER, c->fontContext->VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		/* Render quad */
		glDrawArrays(GL_TRIANGLES, 0, 6);
		/* Now advance cursors for next glyph */
		x += (ch.Advance >> 6) * scale; /* Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64) */
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D,0);
}

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
	glUseProgram(c->fontContext->fontShaderID);
	VOX_PROTECTED_LOG(c, CYAN"Font shader id: %d\n"RESET, c->fontContext->fontShaderID);
	if (!freeTypeFontLoadChar(c, face)) {
		return ;
	}
	/* Free the freetype rsc */
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glm_ortho(0.0f, (f32)SCREEN_WIDTH, 0.0f, (f32)SCREEN_HEIGHT, -1.0f, 1.0f, c->fontContext->projection);
	set_shader_var_mat4(c->fontContext->fontShaderID, "projection", c->fontContext->projection);
	set_shader_texture(c->fontContext->fontShaderID, c->fontContext->font[0].TextureID, GL_TEXTURE_2D, "text");
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
	glUseProgram(c->cubeShaderID);
	handle_input(c);
	update_camera(c, c->cubeShaderID);
	glUseProgram(0);
	mtx_unlock(&c->gameMtx);
}

void fpsDisplayOnScreen(Context *c) {
	renderFontText(c, "FPS: ", 15.0f, (f32)SCREEN_HEIGHT - 25.0f, 0.4f, (vec3){1.0, 1.0, 0.0});
	char *fps = ft_itoa(fpsGet());
	renderFontText(c, (const char *)fps, 80.0f, (f32)SCREEN_HEIGHT - 25.0f, 0.4f, (vec3){1.0, 0.7, 0.0});
	free(fps);
}

void renderGame(Context *c, GLuint skyTexture) {
	/* Update render */
	renderChunksVBOhandling(c);
	/* Clear the color and depth buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* Render logic */
	displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam.projection, c->cam.view);
	chunksRender(c, c->cubeShaderID);
	fpsDisplayOnScreen(c);
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