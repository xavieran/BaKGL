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
out vec3 Position_worldspace;
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
uniform mat4 M;
uniform Light light;
uniform vec3 cameraPosition_worldspace;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);
	
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
	EyeDirection_cameraspace = normalize(vec3(0, 0, 0) - vertexPosition_cameraspace);

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	lightDirection_cameraspace = -1 * normalize((V * vec4(light.mDirection, 0)).xyz);
	
	// Normal of the the vertex, in camera space
    // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	Normal_cameraspace = normalize((V * M * vec4(vertexNormal_modelspace, 0)).xyz);
	
	vertexColor = vertexColor_modelspace;
    uvCoords = textureCoords.xyz;
    texBlend = texBlendVec;

    DistanceFromCamera = distance(Position_worldspace, cameraPosition_worldspace);
}
