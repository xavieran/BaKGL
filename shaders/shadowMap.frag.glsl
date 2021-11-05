#version 330 core

in float texBlend;
in vec4 vertexColor;
in vec3 uvCoords;

uniform sampler2DArray texture0;

void main()
{
    vec4 textureSample = texture(texture0, uvCoords);
    float textureAlpha = textureSample.a;
    float materialAlpha = vertexColor.a;
    float alpha       = mix(materialAlpha, textureAlpha, texBlend);

    if (alpha == 0) discard;
}  
