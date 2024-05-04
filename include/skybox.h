#ifndef HEADER_SKYBOX_H
#define HEADER_SKYBOX_H

#include "vox.h"

GLuint skyboxInit();
void displaySkybox(GLuint skyboxVAO, GLuint skyboxTexture, GLuint skyboxShader, mat4 projection, mat4 view);

#endif /* HEADER_SKYBOX_H */
