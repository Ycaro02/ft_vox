#version 330 core

out vec4 FragColor;

in vec3 TexCoord;

uniform sampler3D textureAtlas;

const float fogStart = 160.0;
const float fogEnd = 180.0;
const vec3 fogColor = vec3(0.5, 0.5, 0.5);

void main()
{
    vec4 baseColor = texture(textureAtlas, TexCoord);
    
    float depth = gl_FragCoord.z / gl_FragCoord.w;
    
    float fogFactor = smoothstep(fogStart, fogEnd, depth);
    vec4 finalColor = mix(baseColor, vec4(fogColor, 1.0), fogFactor);
    FragColor = finalColor;
}