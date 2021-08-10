#version 330 core

in vec3 Position_screenspace;
in vec3 uvCoords;

// Ouput data
out vec4 color;

uniform int  colorMode;
uniform vec4 blockColor;
uniform sampler2DArray texture0;

void main()
{
    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    vec3 blockColorB   = blockColor.xyz;
    if (colorMode == 1) // block mode
        color = blockColor;
    else if (colorMode == 2) // tint mode
        color = vec4(
            mix(blockColorB, textureColor, .5),
            textureSample.a);
    else
        color = vec4(textureColor, textureSample.a);
}
