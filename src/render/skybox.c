#include "../../include/skybox.h"


GLuint skyboxInit()
{
	    float skyboxVertices[] = {
        // BACK
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // FRONT
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // RIGHT
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // LEFT
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // BOT
        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        // TOP
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
	GLuint skyboxVAO, skyboxVBO;

    // Create a Vertex Array Object (VAO) for the skybox and a Vertex Buffer Object (VBO)
    // to hold vertex data.
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    // Bind the VAO and VBO, then load the skybox vertices into the VBO.
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // Enable the first attribute (0), then define it as 3-component float vertices.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	return (skyboxVAO);
}

void displaySkybox(GLuint skyboxVAO, GLuint skyboxTexture, GLuint skyboxShader, mat4 projection, mat4 view)
{
    // Prepare to draw the skybox: disable depth writing, use the skybox shader, 
    // and set the "skybox" uniform to texture unit 0.
  	glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glUseProgram(skyboxShader);

    // Remove translation from the view matrix to keep the skybox centered around 
    // the camera position.
    mat3 view3x3;
    glm_mat4_pick3(view, view3x3);
    
    mat4 skyView;
    glm_mat4_copy(GLM_MAT4_ZERO, skyView);
    glm_mat4_ins3(view3x3, skyView);

    // Set the "view" and "projection" uniforms in the skybox shader.
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, (GLfloat *)&skyView[0]);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, (GLfloat *)&projection[0]);
    
    // Bind the VAO, set the active texture to texture unit 0, bind the skybox 
    // cubemap texture, then draw the skybox.
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}