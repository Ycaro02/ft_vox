#include "include/glad/gl.h"
#include "include/glfw3/glfw3.h"
#include "include/perlin_noise.h"
#include "include/vox.h"

#define WIDTH 800
#define HEIGHT 600

GLuint BRUT_shaderinit()
{
	const char *vertex_shader_source = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char *fragment_shader_source = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform sampler2D texture1;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(texture1, vec2(gl_FragCoord.x / 800.0, gl_FragCoord.y / 600.0));\n"
		"}\n\0";

	// Create vertex shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	// Check for vertex shader compile errors
	int success;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		ft_printf_fd(2, "Error: Vertex shader compilation failed\n%s\n", info_log);
	}

	// Create fragment shader
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	// Check for fragment shader compile errors
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		ft_printf_fd(2, "Error: Fragment shader compilation failed\n%s\n", info_log);
	}

	// Create shader program
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	// Check for shader program link errors
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		ft_printf_fd(2, "Error: Shader program linking failed\n%s\n", info_log);
	}
	return (shader_program);
}


GLuint draw_image(GLFWwindow *window, int width, int height) {
    // Generate Perlin noise
    float **noise = noiseSample2D(width, height);

    // Convert Perlin noise to image
    GLubyte *image = malloc(width * height * sizeof(GLubyte));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Map Perlin noise from [-1, 1] to [0, 255] for grayscale
            GLubyte color = (GLubyte)((noise[i][j] + 1.0f) * 0.5f * 255);
            image[i * width + j] = color; // Gray
        }
    }

	for (int i = 0; i < height; i++) {
		ft_printf_fd(1, "Color: |%d| |%d| |%d|\n", image[i], image[i + 1], image[i + 2]);
	}

    // Create shader program
    GLuint shader_program = BRUT_shaderinit();
    glUseProgram(shader_program);


    // Create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set texture uniform
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);

    // Create VAO and VBO for rectangle
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // Bind VAO
    glBindVertexArray(vao);

    // Rectangle vertices
    float vertices[] = {
        // Positions
        1.0f,  1.0f, 0.0f,  // Top right
        1.0f, -1.0f, 0.0f,  // Bottom right
       -1.0f, -1.0f, 0.0f,  // Bottom left
       -1.0f,  1.0f, 0.0f   // Top left
    };

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Swap buffers

    // Free memory

}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        ft_printf_fd(2, "Error: Failed to initialize GLFW\n");
        return -1;
    }

    // Create window
    GLFWwindow *window = init_openGL_context();
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Draw image
    GLuint vao = draw_image(window, WIDTH, HEIGHT);
	ft_printf_fd(1, "VAO: |%d|\n", vao);

    // Main loop
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
	    glfwSwapBuffers(window);
		glfwPollEvents();
    }

    // Cleanup and exit
    glfw_destroy(window);
    glfwTerminate();
    return 0;
}
