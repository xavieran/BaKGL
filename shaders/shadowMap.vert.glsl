#version 330 core

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 1) in vec3  vertexNormal_modelspace;
layout(location = 2) in vec4  vertexColor_modelspace;
layout(location = 3) in vec3  textureCoords;
layout(location = 4) in float texBlendVec;

out float texBlend;
out vec3 uvCoords;
out vec4 vertexColor;

uniform mat4 lightSpaceMatrix;
uniform mat4 M;

void main()
{
    gl_Position = lightSpaceMatrix * M * vec4(vertexPosition_modelspace, 1.0);

    vertexColor = vertexColor_modelspace;
    uvCoords = textureCoords.xyz;
    texBlend = texBlendVec;


}  

