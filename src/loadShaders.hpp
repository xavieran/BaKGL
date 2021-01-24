#include "logger.hpp"

#include <GL/glew.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <filesystem>
#include <vector>

#include <stdlib.h>
#include <string.h>

GLuint LoadShaders(
    char const* vertexShader,
    char const* fragmentShader)
{
    auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::cout << "VERTEXSHADER" << vertexShader;

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

std::optional<std::string> FindFile(const std::string& shaderPath)
{   
    auto* home = getenv("HOME");
    if (home == nullptr) return std::optional<std::string>{};

    std::vector<std::string> searchPaths{
        "Code/BaKSandbox/shaders",
        ".shaders"};

    for (const auto& searchPath : searchPaths)
    {
        auto fullPath = std::filesystem::path{home} / searchPath / shaderPath;
        if (std::filesystem::exists(fullPath))
            return fullPath.string();
    }

    return std::optional<std::string>{};
}

// This overload is slightly ambiguous...
GLuint LoadShaders(
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath)
{
    auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    const auto LoadFileContents = [](const auto& path)
    {
        auto fullPath = FindFile(path);
        if (!fullPath) throw std::exception{};

        std::ifstream in{};
        in.open(*fullPath, std::ios::in);

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


