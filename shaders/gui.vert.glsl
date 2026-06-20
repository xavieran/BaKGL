#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 textureCoords;

out vec3 uvCoords;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);
	uvCoords = textureCoords.xyz;
}
