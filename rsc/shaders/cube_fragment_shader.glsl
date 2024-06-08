#version 330 core

out vec4 FragColor;

in vec3 TexCoord;

uniform sampler3D textureAtlas;

const float fogStart = 140.0;
const float fogEnd = 200.0; 
vec4 fogColor = vec4(1.0, 1.0, 1.0, 0.0);

void main()
{
    vec4 baseColor = texture(textureAtlas, TexCoord);
    
	fogColor = vec4(baseColor.xyz, 0.0);

    float depth = gl_FragCoord.z / gl_FragCoord.w;
    

    float fogFactor = smoothstep(fogStart, fogEnd, depth);
    vec4 finalColor = mix(baseColor, fogColor, fogFactor);

    FragColor = finalColor;
}