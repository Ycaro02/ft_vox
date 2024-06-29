#version 330 core
out vec4 FragColor;

#define SKY_TOP_FACE 1.0
#define EPSILON 0.001

in vec3 TexCoords;
in float FaceID;

#define CAMERA_IN_CAVE 2

uniform int camIsUnderground;
uniform samplerCube texture1;

bool isSkyTopface() {
	return (abs(FaceID - SKY_TOP_FACE) < EPSILON);
}

void main()
{ 
	if (camIsUnderground == CAMERA_IN_CAVE && !isSkyTopface()) {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return ;	
	}
    FragColor = texture(texture1, TexCoords);
}