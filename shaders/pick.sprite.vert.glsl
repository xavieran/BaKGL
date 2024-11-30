#version 330 core

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 1) in vec3  vertexNormal_modelspace;
layout(location = 2) in vec4  vertexColor_modelspace;
layout(location = 3) in vec3  textureCoords;
layout(location = 4) in float texBlendVec;

out vec3 uvCoords;
out float texBlend;

uniform mat4 MVP;
uniform mat4 M;
uniform vec3 cameraPosition_worldspace;

void main(){
    vec3 vertexPosition_worldspace = (M * vec4(vertexPosition_modelspace, 1.0)).xyz;
    vec3 direction_toCamera = normalize(cameraPosition_worldspace - vertexPosition_worldspace);
    mat4 billboardRotationMatrix;

    billboardRotationMatrix[0] = vec4(direction_toCamera.x, 0.0, direction_toCamera.z, 0.0);
    billboardRotationMatrix[1] = vec4(0.0, 1.0, 0.0, 0.0);
    billboardRotationMatrix[2] = vec4(-direction_toCamera.z, 0.0, direction_toCamera.x, 0.0);
    billboardRotationMatrix[3] = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 billboardPosition = billboardRotationMatrix * vec4(vertexPosition_modelspace, 1.0);

    gl_Position = MVP * billboardPosition;

    uvCoords = textureCoords.xyz;
    texBlend = texBlendVec;
}
