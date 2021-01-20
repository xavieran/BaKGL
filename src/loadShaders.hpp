#include "logger.hpp"

#include <GL/glew.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include <stdlib.h>
#include <string.h>

static const char* vertexShader = R"(
#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec4 vertexColor_modelspace;
layout(location = 3) in vec3 textureCoords;
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
uniform vec3 lightPosition_worldspace;
uniform vec3 CameraPosition_worldspace;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 lightPosition_cameraspace = ( V * vec4(lightPosition_worldspace,1)).xyz;
	lightDirection_cameraspace = lightPosition_cameraspace + EyeDirection_cameraspace;
	
	// Normal of the the vertex, in camera space
    // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	Normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz; 
	//Normal_cameraspace = (V * transpose(inverse(M)) * vec4(vertexNormal_modelspace,0)).xyz;
	
	vertexColor = vertexColor_modelspace;

    uvCoords = textureCoords.xyz;

    DistanceFromCamera = distance(Position_worldspace, CameraPosition_worldspace);
    texBlend = texBlendVec;
}

)";

static const char* fragmentShader = R"(
#version 330 core

in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 lightDirection_cameraspace;
in vec4 vertexColor;
in vec3 uvCoords;
in float texBlend;
in float DistanceFromCamera;

// Ouput data
out vec4 color;

uniform mat4 MV;
uniform vec3 lightPosition_worldspace;
uniform sampler2DArray texture0;

void main(){

	// light emission properties
	// You probably want to put them as uniforms
	vec3 lightColor = vec3(1,1,1);
	float lightPower = 100000.0f;
	
    float k = .00002;
    vec3 fogColor   = vec3(0.5, 0.5, 0.8);
	float fogFactor = exp(-DistanceFromCamera * k);
    
	vec3 materialDiffuseColor = vertexColor.xyz;
    float materialAlpha = vertexColor.a;

    vec4 textureSample = texture(texture0, uvCoords);
    vec3 textureColor  = textureSample.xyz;
    float textureAlpha = textureSample.a;

    // Choose either vertex color or texture color with the texBlend
    vec3 diffuseColor = mix(materialDiffuseColor, textureColor, texBlend);
    float alpha       = mix(materialAlpha, textureAlpha, texBlend);

    if (alpha == 0) discard;

	//vec3 materialAmbientColor = vec3(0.1,0.1,0.1) * diffuseColor;
	vec3 materialAmbientColor = .4 * diffuseColor;
	vec3 materialSpecularColor = vec3(0.1);

	// Distance to the light
	float distance = length(lightPosition_worldspace - Position_worldspace);

	// Normal of the computed fragment, in camera space
	vec3 n = normalize(Normal_cameraspace);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(lightDirection_cameraspace);
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp(dot(n, l), 0, 1);
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l, n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp(dot(E, R), 0, 1);

    vec3 litColor = 
          materialAmbientColor 
        + diffuseColor * lightColor * lightPower * cosTheta
            / (distance * distance)
		+ materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 4) 
            / (distance * distance);
    
    vec3 foggedColor = mix(fogColor, litColor, fogFactor);

    color = vec4(foggedColor, alpha);
}

)";

GLuint LoadShaders(char const* vertexShader,char const* fragmentShader)
{
    auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	glShaderSource(VertexShaderID, 1, &vertexShader, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		logger.Error() << "VertexShaderError: " << &VertexShaderErrorMessage[0] << std::endl;
	}

	// Compile Fragment Shader
	glShaderSource(FragmentShaderID, 1, &fragmentShader, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		logger.Error() << "FragmentShaderError: " << &FragmentShaderErrorMessage[0] << std::endl;
	}

	// Link the program
    logger.Debug("Linking shader program");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		logger.Error() << "ProgramErrorMessage: " << &ProgramErrorMessage[0] << std::endl;
        throw std::exception{};
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadShaders(
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath)
{
    auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    const auto LoadFileContents = [](const auto& path)
    {
        std::ifstream in{};
        in.open(path, std::ios::in);

        if (!in.good()) throw std::exception{};

        std::string contents{
            (std::istreambuf_iterator<char>(in)),
             std::istreambuf_iterator<char>()};

        in.close();

        return contents;
    };


    const auto vertexShaderCode = LoadFileContents(vertexShaderPath);
    const auto fragmentShaderCode = LoadFileContents(fragmentShaderPath);

    logger.Debug() << "VertexShaderCode: " << vertexShaderCode << std::endl;
    logger.Debug() << "FragmentShaderCode: " << fragmentShaderCode << std::endl;

    return LoadShaders(
        vertexShaderCode.c_str(),
        fragmentShaderCode.c_str());
}


