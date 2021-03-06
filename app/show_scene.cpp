#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "bak/font.hpp"
#include "bak/inputHandler.hpp"
#include "bak/hotspot.hpp"
#include "bak/screens.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/systems.hpp"
#include "bak/textureFactory.hpp"

#include "com/algorithm.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/glfw.hpp"
#include "graphics/plane.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"

#include "gui/cursor.hpp"
#include "gui/gdsScene.hpp"
#include "gui/gui.hpp"
#include "gui/scene.hpp"
#include "gui/textBox.hpp"

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

    //Logging::LogState::Disable("LoadScenes");
    //Logging::LogState::Disable("LoadSceneIndices");

    
    auto guiScalar = 2.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 240.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar;

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show GUI");

    auto guiScale = glm::vec3{guiScalar, guiScalar, 0};
    auto guiScaleInv = glm::vec3{1 / guiScalar, 1 / guiScalar, 0};

    glViewport(0, 0, width, height);

    ImguiWrapper::Initialise(window.get());

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0, 0.0f);

    auto guiShaderProgram = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShader = guiShaderProgram.Compile();

    const auto root = static_cast<std::uint8_t>(std::atoi(argv[1]));
    auto currentSceneRef = BAK::HotspotRef{root, *argv[2]};
    
    auto scenes = std::stack<std::unique_ptr<Gui::GDSScene>>{};
    auto frames = std::stack<std::unique_ptr<Gui::Frame>>{};
    auto dialog = std::optional<std::string_view>{};

    BAK::DialogStore dialogStore{};
    dialogStore.Load();
    const auto GetText = [&](const auto& tgt) -> std::string_view
    {
        try
        {
            return dialogStore.GetSnippet(BAK::KeyTarget{tgt}).GetText();
        }
        catch (const std::runtime_error& e)
        {
            return e.what();
        }
    };

    const auto MakeScene = [&](auto ref){
        scenes.emplace(std::make_unique<Gui::GDSScene>(ref));
        frames.emplace(std::make_unique<Gui::Frame>(
            glm::vec3{0,0,0},
            glm::vec3{320, 240, 0}));

        for (const auto& hs : scenes.top()->mHotspots.mHotspots)
        {
            auto pic = hs.mKeyword - 1;
            auto pos = glm::vec3{hs.mTopLeft.x, hs.mTopLeft.y, 0};
            frames.top()->mChildren.emplace_back(
                false,
                false,
                0,
                pic,
                glm::vec3{pos.x, pos.y, 0},
                glm::vec3{hs.mDimensions.x, hs.mDimensions.y, 0},
                glm::vec3{1,1,1},
                [&, arg=hs.mActionArg1](){
                    if (dialog) dialog.reset();
                    if (hs.mAction == BAK::HotspotAction::GOTO)
                    {
                        char c = static_cast<char>(65 + arg - 1);
                        currentSceneRef = BAK::HotspotRef{root, c};
                        logger.Debug() << "Switching to: " << c << " " 
                            << currentSceneRef.ToFilename() << "\n";
                    }
                    else if (hs.mAction == BAK::HotspotAction::EXIT)
                    {
                        if (scenes.size() > 1)
                        {
                            scenes.pop();
                            frames.pop();
                            currentSceneRef = scenes.top()->mReference;
                        }
                        else
                        {
                            std::exit(0);
                        }
                    }
                },
                [&, tooltip=hs.mTooltip](){
                    if (dialog) dialog.reset();
                    else dialog = GetText(tooltip);
                }
            );
        }
    };

    MakeScene(currentSceneRef);

    auto cursor = Gui::Cursor{};

    const auto fontRenderer = Gui::FontRenderer{"GAME.FNT"};

        
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

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frames.top()->LeftMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frames.top()->LeftMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frames.top()->RightMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            frames.top()->RightMouseRelease(guiScaleInv * click);
        }
    );


    glm::vec3 mousePos;
    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            frames.top()->MouseMoved(guiScaleInv * pos);
            mousePos = pos;
        });

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width/2, height/2);
    // Hide it we will draw one ourselves
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    double pointerPosX, pointerPosY;

    guiShader.UseProgramGL();

    double acc = 0;
    unsigned i = 0;

    do
    {
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        acc += deltaTime;
        if (acc > .2)
        { 
            acc = 0;
        }

        lastTime = currentTime;

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto programId = guiShader.GetHandle();

        GLuint mvpMatrixID   = glGetUniformLocation(programId, "MVP");
        GLuint modelMatrixID = glGetUniformLocation(programId, "M");
        GLuint viewMatrixID  = glGetUniformLocation(programId, "V");
        GLuint blockColorId  = glGetUniformLocation(programId, "blockColor");
        GLuint useColorId    = glGetUniformLocation(programId, "useColor");

        // ...
        int useColor = 0;
        auto blockColor = glm::vec4{0};

        const auto Draw = [&](auto modelMatrix, auto object)
        {
            MVP = viewMatrix * scaleMatrix * modelMatrix;

            guiShader.SetUniform(mvpMatrixID,   MVP);
            guiShader.SetUniform(modelMatrixID, modelMatrix);
            guiShader.SetUniform(viewMatrixID,  viewMatrix);
            guiShader.SetUniform(useColorId,    useColor);
            guiShader.SetUniform(blockColorId,  blockColor);
        
            const auto [offset, length] = object;
            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                length,
                GL_UNSIGNED_INT,
                (void*) (offset * sizeof(GLuint)),
                offset
            );
        };
        
        auto& scene = *scenes.top();
        scene.mSprites.BindGL();

        for (const auto& action : scene.mDrawActions)
        {
            std::visit(overloaded{
                [&](const Gui::SceneSprite& sprite){
                    auto sprScale = glm::scale(glm::mat4{1}, sprite.mScale);
                    auto sprTrans = glm::translate(glm::mat4{1}, sprite.mPosition);
                    modelMatrix = sprTrans * sprScale;
                    auto object = scene.mSprites.Get(sprite.mImage);
                    useColor = 0;
                    Draw(modelMatrix, object);
                },
                [&](const Gui::SceneRect& rect){
                    auto sprScale = glm::scale(glm::mat4{1}, rect.mDimensions);
                    auto sprTrans = glm::translate(
                        glm::mat4{1},
                        rect.mPosition);
                    modelMatrix = sprTrans * sprScale;
                    useColor = 1;
                    blockColor = rect.mColor;
                    Draw(modelMatrix, std::make_pair(0, 6));
                },
                [&](const Gui::EnableClipRegion& clip){
                    glScissor(
                        clip.mBottomLeft.x * guiScale.x,
                        clip.mBottomLeft.y * guiScale.y,
                        clip.mDims.x       * guiScale.x,
                        clip.mDims.y       * guiScale.y);
                    glEnable(GL_SCISSOR_TEST);
                },
                [&](const Gui::DisableClipRegion& clip){
                    glDisable(GL_SCISSOR_TEST);
                }},
                action);
        }

        useColor = 0;
        
        fontRenderer.GetSprites().BindGL();

        auto text = dialog
            ? *dialog 
            : GetText(scenes.top()->mHotspots.mFlavourText);
        const auto titleIt = find_nth(text.begin(), text.end(), '#', 2);
        if (titleIt != text.end())
        {
            const auto titleLen = std::distance(text.begin(), titleIt);
            Gui::TextBox{
                glm::vec3{144.0, 108.0, 0},
                glm::vec3{320 - 16 - 16, 240 - 120, 0}}.Render(
                fontRenderer,
                text.substr(0, titleLen),
                [&](const auto& pos, auto object){
                    modelMatrix = pos;
                    Draw(modelMatrix, object);
                });
            text = text.substr(titleLen, text.size() - titleLen);
        }

        Gui::TextBox{
            glm::vec3{16, 120, 0},
            glm::vec3{320 - 16 - 16, 240 - 120, 0}}.Render(
            fontRenderer,
            text,
            [&](const auto& pos, auto object){
                modelMatrix = pos;
                Draw(modelMatrix, object);
            });

        cursor.GetSprites().BindGL();

        {
            unsigned draw = 0;
            for (const auto& child : frames.top()->mChildren)
                if (child.mHighlighted)
                    draw = child.mPressedImage;

            auto cursorTrans = glm::translate(
                glm::mat4{1},
                mousePos * guiScaleInv);

            modelMatrix = cursorTrans;
            auto object = cursor.GetSprites().Get(draw);
            Draw(modelMatrix, object);
        }

        if (currentSceneRef != scenes.top()->mReference)
            MakeScene(currentSceneRef);

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


