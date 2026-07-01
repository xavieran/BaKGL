#version 330 core

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 3) in vec3  textureCoords;

out vec3 uvCoords;

uniform mat4 MVP;
uniform mat4 M;
uniform vec3 cameraPosition_worldspace;
uniform vec3 billboardCenter;

void main() {
    vec3 centerPosition_worldspace = billboardCenter;

    vec3 direction_toCamera = normalize(
        cameraPosition_worldspace - centerPosition_worldspace);

    mat4 billboardRotationMatrix;
    billboardRotationMatrix[0] =
        vec4(direction_toCamera.z, 0.0, -direction_toCamera.x, 0.0);
    billboardRotationMatrix[1] = vec4(0.0, 1.0, 0.0, 0.0);
    billboardRotationMatrix[2] =
        vec4(direction_toCamera.x, 0.0, direction_toCamera.z, 0.0);
    billboardRotationMatrix[3] = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 billboardPosition =
        billboardRotationMatrix * vec4(vertexPosition_modelspace, 1.0);
    gl_Position = MVP * billboardPosition;

    uvCoords = textureCoords;
}
