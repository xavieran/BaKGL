#version 330 core

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 1) in vec3  vertexNormal_modelspace;
layout(location = 2) in vec4  vertexColor_modelspace;
layout(location = 3) in vec3  textureCoords;
layout(location = 4) in float texBlendVec;

// Output data ; will be interpolated for each fragment.
out vec3 uvCoords;
out float texBlend;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);
    uvCoords = textureCoords.xyz;
    texBlend = texBlendVec;
}
