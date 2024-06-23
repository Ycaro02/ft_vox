#version 330 core

#define PLAIN_BIOME 0

out vec4 FragColor;

in vec3 TexCoord;

flat in int biomeType;

uniform sampler3D textureAtlas;

const float fogStart = 140.0;
const float fogEnd = 200.0; 
vec4 fogColor = vec4(1.0, 1.0, 1.0, 0.0);

void main()
{
    vec4 baseColor = texture(textureAtlas, TexCoord);

	float grayTolerance = 0.01; // Ajustez cette valeur en fonction de vos besoins
    bool isGray = abs(baseColor.r - baseColor.g) < grayTolerance && abs(baseColor.g - baseColor.b) < grayTolerance && abs(baseColor.r - baseColor.b) < grayTolerance;

	

	if (biomeType == PLAIN_BIOME && isGray) {
		vec3 biomeColor = vec3(0.75, 1.0, 0.0);
		baseColor = vec4(baseColor.rgb * biomeColor, baseColor.a);
	}

	// baseColor.w = 1.0; // set base alpha to 1.0

	fogColor = vec4(baseColor.xyz, 0.0);

    float depth = gl_FragCoord.z / gl_FragCoord.w;
    

    float fogFactor = smoothstep(fogStart, fogEnd, depth);
    vec4 finalColor = mix(baseColor, fogColor, fogFactor);

    FragColor = finalColor;
}