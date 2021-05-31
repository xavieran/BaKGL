#include "src/camera.hpp"
#include "src/coordinates.hpp"
#include "src/inputHandler.hpp"
#include "src/logger.hpp"
#include "src/screens.hpp"
#include "src/systems.hpp"

#include "graphics/glfw.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"

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

    auto objStore = Graphics::MeshObjectStorage{};
    auto sphere = Sphere{30.0, 12, 6, true};
    objStore.AddObject(
        "quad",
        Graphics::SphereToMeshObject(sphere, glm::vec4{1.0, 0, 0, .7}));

    auto height = 800;
    auto width = 1400;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show Scene");

    ImguiWrapper::Initialise(window.get());
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto modelShader = ShaderProgram{
        "normal.vert.glsl",
        "normal.frag.glsl"};
    auto modelShaderId = modelShader.Compile();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    BAK::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("textureCoord", 1, 3);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objStore.mVertices);
    buffers.LoadBufferDataGL("textureBlend", GL_ARRAY_BUFFER, objStore.mTextureBlends);

    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objStore.mIndices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    //BAK::TextureBuffer textureBuffer{};
    //textureBuffer.LoadTexturesGL(
    //    textureStore.GetTextures(),
    //    textureStore.GetMaxDim());

    glm::mat4 viewMatrix{1};
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{0};

    double currentTime = 0;
    double lastTime = 0;
    //float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);

    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureBuffer.mTextureBuffer);

    double pointerPosX, pointerPosY;

    do
    {
        currentTime = glfwGetTime();
        //deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        std::cout << lastTime << "\n";

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(modelShaderId);

    ImguiWrapper::Shutdown();

    glfwDestroyWindow(window.get());
    glfwTerminate();

    return 0;
}


