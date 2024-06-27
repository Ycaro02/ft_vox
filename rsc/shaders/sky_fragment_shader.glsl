#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

#define CAMERA_IN_CAVE 2

uniform int camIsUnderground;
uniform samplerCube texture1;

void main()
{ 
	// if (camIsUnderground == 1 || camIsUnderground == 2) {
	if (camIsUnderground == CAMERA_IN_CAVE) {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return ;	
	}
    FragColor = texture(texture1, TexCoords);
}