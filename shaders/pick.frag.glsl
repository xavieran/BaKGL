#version 330 core

in vec3 uvCoords;
in float texBlend;

// Ouput data
out vec4 color;

uniform sampler2DArray texture0;
uniform uint entityId;

void main()
{
    vec4 textureSample = texture(texture0, uvCoords);
    float textureAlpha = textureSample.a;

    if (!(texBlend < 1) && textureAlpha == 0) discard;

    float entityIdR = (entityId & 0xffu);
    float entityIdG = ((entityId >> 8) & 0xffu);
    float entityIdB = ((entityId >> 16) & 0xffu);
    float entityIdA = ((entityId >> 24) & 0xffu);
    color = vec4(entityIdR, entityIdG, entityIdB, entityIdA);
}

