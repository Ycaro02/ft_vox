#ifndef HEADER_TEXT_RENDER_H
#define HEADER_TEXT_RENDER_H


#include <ft2build.h>
#include FT_FREETYPE_H
#include "typedef_struct.h"


#define TEXT_HEIGHT_OFFSET_GET(x) ((f32)SCREEN_HEIGHT - (f32)x)
#define FPS_SCALE 0.3f
#define TEXT_DESCRIBED_WIDTH_OFFSET 15.0f
#define TEXT_DATA_WIDTH_OFFSET 150.0f
#define VEC3_GREEN (vec3){0.0, 1.0, 0.0}
#define VEC3_RED (vec3){1.0, 0.0, 0.0}
#define VEC3_CYAN (vec3){0.0, 1.0, 1.0}
#define VEC3_YELLOW (vec3){1.0, 1.0, 0.0}
#define VEC3_ORANGE (vec3){1.0, 0.7, 0.0}
#define VEC3_WHITE (vec3){1.0, 1.0, 1.0}
#define VEC3_BLACK (vec3){0.0, 0.0, 0.0}
#define VEC3_PINK (vec3){1.0, 0.0, 1.0}

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

void	textRender(Context *c, const char *text, f32 x, f32 y, f32 scale, vec3 color);
s8		freeTypeFontInit(Context *c);

#endif /* HEADER_TEXT_RENDER_H */
