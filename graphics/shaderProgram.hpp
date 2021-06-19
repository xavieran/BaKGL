#pragma once

#include "com/logger.hpp"

#include <GL/glew.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

std::string ShaderTypeToString(GLenum shaderType)
{
    switch (shaderType)
    {
    case GL_VERTEX_SHADER: return "VertexShader";
    case GL_GEOMETRY_SHADER: return "GeometryShader";
    case GL_FRAGMENT_SHADER: return "FragmentShader";
    default: return "UnknownShader";
    }
}

class ShaderProgramHandle
{
public:
    ShaderProgramHandle(GLuint handle)
    :
        mHandle{handle}
    {}

    ShaderProgramHandle& operator=(ShaderProgramHandle&& other)
    {
        mHandle = other.mHandle;
        other.mHandle = 0;
        return *this;
    }

    ShaderProgramHandle(ShaderProgramHandle&& other)
    {
        mHandle = other.mHandle;
        other.mHandle = 0;
    }

    ~ShaderProgramHandle()
    {
        glDeleteProgram(mHandle);
        mHandle = 0;
    }

    ShaderProgramHandle& operator=(const ShaderProgramHandle&) = delete;
    ShaderProgramHandle(const ShaderProgramHandle&) = delete;

    GLuint GetHandle() const
    {
        return mHandle;
    }

private:
    GLuint mHandle;
};

class ShaderProgram
{
public:
    ShaderProgram(
        const std::string& vertexShader,
        const std::string& fragmentShader)
    :
        ShaderProgram(vertexShader, std::optional<std::string>{}, fragmentShader)
    {}

    ShaderProgram(
        const std::string& vertexShader,
        const std::optional<std::string>& geometryShader,
        const std::string& fragmentShader)
    :
        mVertexShader{vertexShader},
        mGeometryShader{geometryShader},
        mFragmentShader{fragmentShader},
        mProgramId{0},
        mLogger{Logging::LogState::GetLogger("ShaderProgram")}
    {}

    ShaderProgramHandle Compile()
    {
        std::vector<GLuint> shaders{};
        shaders.emplace_back(CompileShader(mVertexShader, GL_VERTEX_SHADER));
        if (mGeometryShader)
            shaders.emplace_back(CompileShader(*mGeometryShader, GL_GEOMETRY_SHADER));
        shaders.emplace_back(CompileShader(mFragmentShader, GL_FRAGMENT_SHADER));

        mLogger.Debug("Linking shader program");

        GLuint programId = glCreateProgram();
        for (auto shaderId : shaders)
            glAttachShader(programId, shaderId);

        glLinkProgram(programId);
        
        GLint result = GL_FALSE;
        int infoLogLength = 0;
        glGetProgramiv(programId, GL_LINK_STATUS, &result);
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0 || result == GL_FALSE)
        {
            std::vector<char> programErrorMessage(infoLogLength + 1);
            glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
            mLogger.Error() << "ProgramErrorMessage: " << &programErrorMessage[0] << std::endl;
            throw std::runtime_error("Could not link shader program");
        }

        for (auto shaderId : shaders)
        {
            glDetachShader(programId, shaderId);
            glDeleteShader(shaderId);
        }
        
        mProgramId = programId;

        return ShaderProgramHandle{programId};
    }

    GLuint GetProgramId() const { return mProgramId; }

private:

    GLuint CompileShader(const std::string& shader, GLenum shaderType)
    {
        auto shaderId = glCreateShader(shaderType);

        GLint result = GL_FALSE;
        int infoLogLength = 0;
        auto shaderFile = LoadFileContents(shader);
        char const* shaderSrc = shaderFile.c_str();
        glShaderSource(shaderId, 1, &shaderSrc, nullptr);
        glCompileShader(shaderId);

        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0 || result == GL_FALSE)
        {
            std::vector<char> shaderErrorMessage(infoLogLength + 1);
            glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
            mLogger.Error() << ShaderTypeToString(shaderType) << "Error: " 
                << &shaderErrorMessage[0] << std::endl;
            throw std::runtime_error("Could not compile shader");
        }

        return shaderId;

    }

    std::optional<std::string> FindFile(const std::string& shaderPath)
    {   
        auto* home = getenv("HOME");
        if (home == nullptr) return std::optional<std::string>{};

        std::vector<std::string> searchPaths{
            "Code/BaKGL/shaders",
            ".shaders"};

        for (const auto& searchPath : searchPaths)
        {
            auto fullPath = std::filesystem::path{home} / searchPath / shaderPath;
            if (std::filesystem::exists(fullPath))
                return fullPath.string();
        }

        return std::optional<std::string>{};
    }

    std::string LoadFileContents(const std::string& path)
    {
        auto fullPath = FindFile(path);
        if (!fullPath)
        {
            std::stringstream err{};
            err << "Could not find file in search directories " << path << std::endl;
            throw std::runtime_error{err.str()};
        }

        std::ifstream in{};
        in.open(*fullPath, std::ios::in);

        if (!in.good())
        {
            std::stringstream err{};
            err << "Could not open file: " << *fullPath << std::endl;
            throw std::runtime_error{err.str()};
        }

        std::string contents{
            (std::istreambuf_iterator<char>(in)),
             std::istreambuf_iterator<char>()};

        in.close();

        return contents;
    }

    std::string mVertexShader;
    std::optional<std::string> mGeometryShader;
    std::string mFragmentShader;

    GLuint mProgramId;

    const Logging::Logger& mLogger;
};
