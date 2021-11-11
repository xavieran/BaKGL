#pragma once

#include "com/logger.hpp"
#include "com/strongType.hpp"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using Float = StrongType<float, struct FloatTag>;

std::string ShaderTypeToString(GLenum shaderType);

class ShaderProgramHandle
{
public:
    ShaderProgramHandle(GLuint handle);
    ShaderProgramHandle& operator=(const ShaderProgramHandle&) = delete;
    ShaderProgramHandle(const ShaderProgramHandle&) = delete;

    ShaderProgramHandle& operator=(ShaderProgramHandle&& other);
    ShaderProgramHandle(ShaderProgramHandle&& other);
    ~ShaderProgramHandle();
    
    void UseProgramGL() const;

    static void SetUniform(GLuint id, const glm::mat4& value);

    static void SetUniform(GLuint id, int value);
    static void SetUniform(GLuint id, Float value);
    static void SetUniform(GLuint id, const glm::vec3& value);
    static void SetUniform(GLuint id, const glm::vec4& value);
    
    GLuint GetUniformLocation(const std::string& name) const;
    GLuint GetHandle() const;

private:
    GLuint mHandle;
};

class ShaderProgram
{
public:
    ShaderProgram(
        const std::string& vertexShader,
        const std::string& fragmentShader);

    ShaderProgram(
        const std::string& vertexShader,
        const std::optional<std::string>& geometryShader,
        const std::string& fragmentShader);

    ShaderProgramHandle Compile();

    GLuint GetProgramId() const;

private:
    GLuint CompileShader(const std::string& shader, GLenum shaderType);

    std::optional<std::string> FindFile(const std::string& shaderPath);

    std::string LoadFileContents(const std::string& path);


    std::string mVertexShader;
    std::optional<std::string> mGeometryShader;
    std::string mFragmentShader;

    GLuint mProgramId;

    const Logging::Logger& mLogger;
};
