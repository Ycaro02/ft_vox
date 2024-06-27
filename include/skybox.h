#ifndef HEADER_SKYBOX_H
#define HEADER_SKYBOX_H

#include "typedef_struct.h"

GLuint skyboxInit();
void displaySkybox(GLuint skyboxVAO, GLuint skyboxTexture, GLuint skyboxShader, mat4 projection, mat4 view, u8 isUnderground);

#endif /* HEADER_SKYBOX_H */
