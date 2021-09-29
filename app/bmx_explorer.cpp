#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "graphics/inputHandler.hpp"
#include "com/logger.hpp"
#include "bak/screens.hpp"
#include "bak/systems.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/glm.hpp"
#include "graphics/glfw.hpp"
#include "graphics/quad.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
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

    auto width = 640;
    auto height = 480;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BMX Explorer");

    glViewport(0, 0, width, height);

    ImguiWrapper::Initialise(window.get());
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto guiShader = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShaderId = guiShader.Compile();
    
    //auto textures = BAK::TextureFactory::MakeTextureStore("G_LAMUT.BMX", "G_LAMUT.PAL");
    auto textures = Graphics::TextureStore{};
    //BAK::TextureFactory::AddToTextureStore(textures, "CAST.BMX", "INVENTOR.PAL");
    BAK::TextureFactory::AddScreenToTextureStore(textures, "ENCAMP.SCX", "OPTIONS.PAL");
    BAK::TextureFactory::AddToTextureStore(textures, "BICONS1.BMX", "OPTIONS.PAL");
    BAK::TextureFactory::AddToTextureStore(textures, "BICONS2.BMX", "OPTIONS.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "POINTER.BMX", "Z01.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "POINTERG.BMX", "Z01.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "COMPASS.BMX", "Z01.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "MAPICONS.BMX", "Z01.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "C11A2.BMX", "C11A.PAL");
    //BAK::TextureFactory::AddToTextureStore(textures, "C11B.BMX", "C11B.PAL");

    Graphics::TextureBuffer textureBuffer{};
    textureBuffer.LoadTexturesGL(
        textures.GetTextures(),
        textures.GetMaxDim());

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    auto objStore = Graphics::QuadStorage{};
    for (unsigned i = 0; i < textures.GetTextures().size(); i++)
    {
        const auto& tex = textures.GetTexture(i);
        objStore.AddObject(
            Graphics::Quad{
                static_cast<double>(tex.GetWidth()),
                static_cast<double>(tex.GetHeight()),
                static_cast<double>(textures.GetMaxDim()),
                i});
    }

    Graphics::GLBuffers buffers{};
    buffers.AddBuffer("vertex", 0, 3);
    buffers.AddBuffer("textureCoord", 1, 3);

    buffers.LoadBufferDataGL("vertex", GL_ARRAY_BUFFER, objStore.mVertices);
    buffers.LoadBufferDataGL("textureCoord", GL_ARRAY_BUFFER, objStore.mTextureCoords);
    buffers.LoadBufferDataGL(buffers.mElementBuffer, GL_ELEMENT_ARRAY_BUFFER, objStore.mIndices);
    buffers.BindArraysGL();
    glBindVertexArray(0);

    glm::mat4 scaleMatrix = glm::scale(glm::mat4{1}, glm::vec3{width/320., height/240., 0.});
    glm::mat4 viewMatrix = glm::ortho(
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height),
        0.0f,
        -1.0f,
        1.0f);  
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{1};

    int picture = 0;

    Graphics::InputHandler inputHandler{};
    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);
    inputHandler.Bind(GLFW_KEY_W, [&]{ modelMatrix = glm::translate(modelMatrix, {0, 50.0/60, 0}); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ modelMatrix = glm::translate(modelMatrix, {0, -50.0/60, 0}); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ modelMatrix = glm::translate(modelMatrix, {-50.0/60, 0, 0}); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ modelMatrix = glm::translate(modelMatrix, {50.0/60, 0, 0}); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{ scaleMatrix = glm::scale(scaleMatrix, {.9, .9, 0}); });
    inputHandler.Bind(GLFW_KEY_E, [&]{ scaleMatrix = glm::scale(scaleMatrix, {1.1, 1.1, 0}); });
    inputHandler.Bind(GLFW_KEY_RIGHT, [&]{ picture += 1; picture %= textures.GetTextures().size(); logger.Debug() << "Pic: " << picture << "\n";});
    inputHandler.Bind(GLFW_KEY_LEFT, [&]{ picture -= 1; picture %= textures.GetTextures().size(); logger.Debug() << "Pic: " << picture << "\n";});
    inputHandler.BindMouse(GLFW_MOUSE_BUTTON_LEFT, [&](auto p)
    {
        logger.Debug() << p << "\n";
    },
    [](auto){});

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
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
        if (acc > .2)
        { 
            i = (i + 1) % textures.GetTextures().size();
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

        GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
        GLuint modelMatrixID = glGetUniformLocation(programId, "M");
        GLuint viewMatrixID  = glGetUniformLocation(programId, "V");


        MVP = viewMatrix * modelMatrix * scaleMatrix;

        glUniformMatrix4fv(mvpMatrixID,   1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixID,  1, GL_FALSE, glm::value_ptr(viewMatrix));
        
        const auto [offset, length] = objStore.GetObject(picture);
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


