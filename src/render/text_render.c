#include "../../include/text_render.h"
#include "../../include/world.h"
#include "../../include/camera.h"
#include "../../include/shader_utils.h"
#include "../../include/vox.h"

void textRender(Context *c, const char *text, f32 x, f32 y, f32 scale, vec3 color) {
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
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows\
		, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
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

s8 freeTypeFontInit(Context *c) {
	FT_Library	ft;
	FT_Face		face;

	if (FT_Init_FreeType(&ft)) {
		ft_printf_fd(2, "ERROR::FREETYPE: Could not init FreeType Library\n");
		return (FALSE);
	}
	if (FT_New_Face(ft, "rsc/font/arial.ttf", 0, &face)) {
		ft_printf_fd(2, "ERROR::FREETYPE: Failed to load font\n");
		return (FALSE);
	}
	FT_Set_Pixel_Sizes(face, 0, 48);

	if (!(c->fontContext = ft_calloc(sizeof(FontContext), 1))) {
		ft_printf_fd(2, "ERROR::FREETYPE: Failed to allocate memory for font context\n");
		return (FALSE);
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
	if (!freeTypeFontLoadChar(c, face)) {
		return (FALSE);
	}
	/* Free the freetype rsc */
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glm_ortho(0.0f, (f32)SCREEN_WIDTH, 0.0f, (f32)SCREEN_HEIGHT, -1.0f, 1.0f, c->fontContext->projection);
	set_shader_var_mat4(c->fontContext->fontShaderID, "projection", c->fontContext->projection);
	set_shader_texture(c->fontContext->fontShaderID, c->fontContext->font[0].TextureID, GL_TEXTURE_2D, "text");

	return (TRUE);
}