#version 330 core

in vec3 Position_screenspace;
in vec3 uvCoords;

// Ouput data
out vec4 color;

uniform mat4 MV;
uniform sampler2DArray texture0;

void main()
{
    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    float textureAlpha = textureSample.a;

    //if (textureAlpha == 0) discard;

    //color = vec4(1,0,0,1);
    color = vec4(textureColor, textureAlpha);
}

