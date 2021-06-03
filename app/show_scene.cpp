#include "src/camera.hpp"
#include "src/coordinates.hpp"
#include "src/inputHandler.hpp"
#include "src/logger.hpp"
#include "src/screens.hpp"
#include "src/systems.hpp"

#include "graphics/glfw.hpp"
#include "graphics/plane.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"

#include "imgui/imguiWrapper.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"
#include "xbak/PaletteResource.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>
#include <memory>
#include <sstream>

#include <getopt.h>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    BAK::DialogStore dialogStore{};
    dialogStore.Load();

    auto height = 800;
    auto width = 1400;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show Scene");

    glViewport(0, 0, width, height);

    ImguiWrapper::Initialise(window.get());
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto guiShader = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShaderId = guiShader.Compile();

    const auto textures = Graphics::Texture{"BICONS2.BMX", "OPTIONS.PAL"};
    //const auto textures = Graphics::Texture{"G_NORTHW.BMX", "G_NORTHW.PAL"};

    BAK::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(
        textures.GetTextures(),
        textures.GetMaxDim());

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    auto objStore = Graphics::QuadStorage{};
    for (unsigned i = 0; i < textures.GetTextures().size(); i++)
    {
        auto guiScale = 8;
        const auto& tex = textures.GetTexture(i);
        objStore.AddObject(
            Graphics::Quad{
                static_cast<double>(tex.GetWidth()) / 320, //(width / guiScale),
                static_cast<double>(tex.GetHeight()) / 240, //(height / guiScale),
                static_cast<double>(textures.GetMaxDim()) / 320, // (width / guiScale),
                static_cast<double>(textures.GetMaxDim()) / 240, // (height / guiScale),
                i});
    }

    BAK::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("textureCoord", 1, 3);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objStore.mVertices);
    buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, objStore.mTextureCoords);
    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objStore.mIndices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    glm::mat4 viewMatrix{1};
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{0};

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);

    double pointerPosX, pointerPosY;

    glUseProgram(guiShaderId.GetHandle());
    double acc = 0;
    unsigned i = 0;
    do
    {
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        acc += deltaTime;
        if (acc > 1)
        { 
            i = i + 1 % textures.GetTextures().size();
            acc = 0;
        }

        lastTime = currentTime;

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);

        glBindVertexArray(VertexArrayID);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto programId = guiShaderId.GetHandle();
        GLuint textureID = glGetUniformLocation(programId, "texture0");
        glUniform1i(textureID, 0);

        //viewMatrix = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.01f, 1.0f);

        GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
        GLuint modelMatrixID = glGetUniformLocation(programId, "M");
        GLuint viewMatrixID  = glGetUniformLocation(programId, "V");

        MVP = viewMatrix;

        glUniformMatrix4fv(mvpMatrixID,   1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixID,  1, GL_FALSE, glm::value_ptr(viewMatrix));

        const auto [offset, length] = objStore.GetObject(i);
        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            length,
            GL_UNSIGNED_INT,
            (void*) (offset * sizeof(GLuint)),
            offset
        );

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


