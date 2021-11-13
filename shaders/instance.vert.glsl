#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 transform;
layout(location = 3) in ivec4 params;

// params == ivec4(
//     colorMode,
//     texCoordOffset,
//     depth,
//     textureSampler)

out vec3 Position_worldspace;
out vec3 uvCoords;
flat out int colorMode;
out vec4 blockColor;

uniform samplerBuffer texCoords;
uniform mat4 V;

void main(){
    int colorMode = params.x;
    int texCoord = params.y;
    float depth = params.z;
    int sampler = params.w;

    mat4 M = mat4(0);
    M[0] = vec4(transform.z, 0, 0, 0);
    M[1] = vec4(0, transform.w, 0, 0);
    M[2] = vec4(0,0,1,0);
    M[3] = vec4(transform.x, transform.y, 0, 1);
     
	gl_Position = V * M * vec4(vertexPosition_modelspace, 1);
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
    vec4 uv = texelFetch(texCoords, gl_VertexID + texCoord * 6);
    uvCoords = uv.xyz;
    blockColor = color;
}
