#version 330 core

in vec3 Position_screenspace;
in vec3 uvCoords;

// Ouput data
out vec4 color;

uniform int  useColor;
uniform vec4 blockColor;
uniform sampler2DArray texture0;

void main()
{
    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    float textureAlpha = textureSample.a;

    if (useColor != 0)
        color = blockColor;
    else
        color = vec4(textureColor, textureAlpha);
}
