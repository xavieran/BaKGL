#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "bak/inputHandler.hpp"
#include "bak/hotspot.hpp"
#include "bak/screens.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/systems.hpp"
#include "bak/textureFactory.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/glfw.hpp"
#include "graphics/plane.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"

#include "gui/gui.hpp"
#include "gui/scene.hpp"

#include "imgui/imguiWrapper.hpp"

#include "xbak/FileManager.h"
#include "xbak/FileBuffer.h"
#include "xbak/PaletteResource.h"
#include "xbak/RequestResource.h"

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

    auto width = 320.0f * 2;
    auto height = 240.0f * 2;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show GUI");

    auto guiScale = glm::vec3{width / 320, height / 240, 0};
    auto guiScaleInv = glm::vec3{320 / width, 240 / height, 0};

    glViewport(0, 0, width, height);

    ImguiWrapper::Initialise(window.get());

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0, 0.0f);

    auto guiShader = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShaderId = guiShader.Compile();

    auto hotspots = BAK::SceneHotspots{};
    auto fb = FileBufferFactory::CreateFileBuffer(argv[1]);
    hotspots.Load(fb);

    logger.Debug() << "ADS: " << hotspots.mSceneADS
        << " TTM: " << hotspots.mSceneTTM
        << " " << hotspots.mSceneIndex1
        << " " << hotspots.mSceneIndex2 << "\n";

    auto fb2 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneADS);
    auto sceneIndices = BAK::LoadSceneIndices(fb2);
    auto fb3 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneTTM);
    auto scenes = BAK::LoadScenes(fb3);

    //logger.Debug() << "Hotspot index: " << hotspots.mSceneIndex << " ADS Index: " 
    //    << sceneIndex.mSceneTag << " " << sceneIndex.mSceneIndex << "\n";
    const auto& scene1 = scenes[sceneIndices[hotspots.mSceneIndex1].mSceneIndex];
    const auto& scene2 = scenes[sceneIndices[hotspots.mSceneIndex2].mSceneIndex];

    auto textures = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(
        textures, "DIALOG.SCX", "OPTIONS.PAL");

    auto frame = Gui::Frame{
        glm::vec3{0,0,0},
        glm::vec3{320, 240, 0}};

    std::unordered_map<unsigned, unsigned> offsets{};
    std::vector<Gui::DrawingAction> drawActions{};
    drawActions.emplace_back(
        Gui::SceneSprite{
            0,
            glm::vec3{0},
            glm::vec3{1}});

    for (const auto& scene : {scene1, scene2})
    {
        for (const auto& [imageKey, imagePal] : scene.mImages)
        {
            const auto& [image, palKey] = imagePal;
            const auto& palette = scene.mPalettes.find(palKey)->second;
            offsets[imageKey] = textures.GetTextures().size();
            BAK::TextureFactory::AddToTextureStore(
                textures,
                image,
                palette);
        }

        for (const auto& action : scene.mActions)
        {
            drawActions.emplace_back(
                std::visit(overloaded{
                    [&](const BAK::DrawSprite& sa) -> Gui::DrawingAction {
                        return Gui::ConvertSceneAction(
                            sa,
                            textures,
                            offsets);
                    },
                    [](auto&& a) -> Gui::DrawingAction {
                        return Gui::ConvertSceneAction(a);
                    }
                },
                action));
        }
    }

    auto gdsOff = textures.GetTextures().size();
    BAK::TextureFactory::AddToTextureStore(
        textures,
        "POINTERG.BMX",
        "OPTIONS.PAL");

    auto& elements = frame.mChildren;
    for (const auto& hs : hotspots.mHotspots)
    {
        auto pic = hs.mKeyword - 1;
        //assert(elements.size() >= 2);
        auto pos = glm::vec3{hs.mTopLeft.x, hs.mTopLeft.y, 0};
            //+ elements[1].mPosition;
        logger.Debug() << "Add HS: " << pic << " " << pos << "\n";
        elements.emplace_back(
            0,
            false,
            gdsOff,
            pic + gdsOff,
            glm::vec3{pos.x, pos.y, 0},
            glm::vec3{hs.mDimensions.x, hs.mDimensions.y, 0},
            glm::vec3{1,1,1});
    }

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

    glm::mat4 scaleMatrix = glm::scale(glm::mat4{1}, guiScale);
    glm::mat4 viewMatrix = glm::ortho(
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height),
        0.0f,
        -1.0f,
        1.0f);  
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 MVP{1};

    InputHandler inputHandler{};

    InputHandler::BindMouseToWindow(window.get(), inputHandler);
    InputHandler::BindKeyboardToWindow(window.get(), inputHandler);

    inputHandler.Bind(GLFW_KEY_W, [&]{ modelMatrix = glm::translate(modelMatrix, {0, 50.0/60, 0}); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ modelMatrix = glm::translate(modelMatrix, {0, -50.0/60, 0}); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ modelMatrix = glm::translate(modelMatrix, {-50.0/60, 0, 0}); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ modelMatrix = glm::translate(modelMatrix, {50.0/60, 0, 0}); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{ scaleMatrix = glm::scale(scaleMatrix, {.9, .9, 0}); });
    inputHandler.Bind(GLFW_KEY_E, [&]{ scaleMatrix = glm::scale(scaleMatrix, {1.1, 1.1, 0}); });

    inputHandler.BindMouse(GLFW_MOUSE_BUTTON_LEFT,
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frame.MousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frame.MouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            frame.MouseMoved(guiScaleInv * pos);
        });

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    textureBuffer.BindGL();

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

        GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
        GLuint modelMatrixID = glGetUniformLocation(programId, "M");
        GLuint viewMatrixID  = glGetUniformLocation(programId, "V");

        const auto Draw = [&](auto modelMatrix, auto object)
        {
            MVP = viewMatrix * scaleMatrix * modelMatrix;

            glUniformMatrix4fv(mvpMatrixID,   1, GL_FALSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(viewMatrixID,  1, GL_FALSE, glm::value_ptr(viewMatrix));
        
            const auto [offset, length] = objStore.GetObject(object);
            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                length,
                GL_UNSIGNED_INT,
                (void*) (offset * sizeof(GLuint)),
                offset
            );
        };

        for (const auto& action : drawActions)
        {
            std::visit(overloaded{
                [&](const Gui::SceneSprite& sprite){
                    auto sprScale = glm::scale(glm::mat4{1}, sprite.mScale);
                    auto sprTrans = glm::translate(glm::mat4{1}, sprite.mPosition);
                    modelMatrix = sprTrans * sprScale;
                    Draw(modelMatrix, sprite.mImage);
                },
                [&](const Gui::EnableClipRegion& clip){
                    glScissor(
                        clip.mBottomLeft.x * guiScale.x,
                        clip.mBottomLeft.y * guiScale.y,
                        clip.mDims.x * guiScale.x,
                        clip.mDims.y * guiScale.y);
                    glEnable(GL_SCISSOR_TEST);
                },
                [&](const Gui::DisableClipRegion& clip){
                    glDisable(GL_SCISSOR_TEST);
                }},
                action);
        }

        for (const auto& [action, pressed, image, pImage, pos, dim, scale] : elements)
        {
            auto sprScale = glm::scale(glm::mat4{1}, scale);
            auto sprTrans = glm::translate(glm::mat4{1}, pos);
            modelMatrix = sprTrans * sprScale;
            auto sel = pressed ? pImage : image;
            Draw(modelMatrix, sel);
        }

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


