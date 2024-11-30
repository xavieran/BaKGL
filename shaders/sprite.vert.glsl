#version 330 core

struct Light
{
    vec3 mDirection;
    vec3 mAmbientColor;
    vec3 mDiffuseColor;
    vec3 mSpecularColor;
};

layout(location = 0) in vec3  vertexPosition_modelspace;
layout(location = 1) in vec3  vertexNormal_modelspace;
layout(location = 2) in vec4  vertexColor_modelspace;
layout(location = 3) in vec3  textureCoords;
layout(location = 4) in float texBlendVec;

// Output data ; will be interpolated for each fragment.
out vec3 Position_lightSpace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 lightDirection_cameraspace;

out vec4 vertexColor;
out vec3 uvCoords;
out float texBlend;
out float DistanceFromCamera;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M; // This is the model matrix for the object.
uniform Light light;
uniform vec3 cameraPosition_worldspace; // Camera position in world space.
uniform mat4 lightSpaceMatrix;

void main() {
    // Compute the vector from the camera to the vertex in world space.
    vec3 vertexPosition_worldspace = (M * vec4(vertexPosition_modelspace, 1.0)).xyz;
    vec3 direction_toCamera = normalize(cameraPosition_worldspace - vertexPosition_worldspace);

    // Create a rotation matrix that aligns the quad to the camera.
    mat4 billboardRotationMatrix;

    billboardRotationMatrix[0] = vec4(direction_toCamera.x, 0.0, direction_toCamera.z, 0.0);
    billboardRotationMatrix[1] = vec4(0.0, 1.0, 0.0, 0.0);
    billboardRotationMatrix[2] = vec4(-direction_toCamera.z, 0.0, direction_toCamera.x, 0.0);
    billboardRotationMatrix[3] = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 billboardPosition = billboardRotationMatrix * vec4(vertexPosition_modelspace, 1.0);

    gl_Position = MVP * billboardPosition;

    Position_lightSpace = (lightSpaceMatrix * vec4(vertexPosition_worldspace, 1.0)).xyz;

    vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1.0)).xyz;
    EyeDirection_cameraspace = normalize(vec3(0, 0, 0) - vertexPosition_cameraspace);

    lightDirection_cameraspace = -1 * normalize((V * vec4(light.mDirection, 0)).xyz);
    
    Normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace, 0)).xyz);
    
    vertexColor = vertexColor_modelspace;
    uvCoords = textureCoords.xyz;
    texBlend = texBlendVec;

    DistanceFromCamera = distance(vertexPosition_worldspace, cameraPosition_worldspace);
}

