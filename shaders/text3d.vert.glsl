#version 330 core

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 3) in vec3  textureCoords;

out vec3 uvCoords;

uniform mat4 VP;
uniform vec3 cameraPosition_worldspace;
uniform vec3 billboardCenter;
uniform vec2 glyphOffset;
uniform vec2 glyphSize;

void main() {
    vec3 toCamera = cameraPosition_worldspace - billboardCenter;

    vec2 toCamera_xz = toCamera.xz;
    float xzLength = length(toCamera_xz);
    vec3 right = xzLength > 0.0001
        ? vec3(toCamera_xz.y, 0.0, -toCamera_xz.x) / xzLength
        : vec3(1.0, 0.0, 0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);

    vec2 glyphPosition = (vertexPosition_modelspace.xy * glyphSize + glyphOffset)
        * length(toCamera);

    vec3 position_worldspace = billboardCenter
        + right * glyphPosition.x
        + up * glyphPosition.y;

    gl_Position = VP * vec4(position_worldspace, 1.0);

    uvCoords = textureCoords;
}
