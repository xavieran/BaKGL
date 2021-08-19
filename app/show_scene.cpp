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
#include "graphics/guiRenderer.hpp"
#include "graphics/plane.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"

#include "gui/clickButton.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/gdsScene.hpp"
#include "gui/hotspot.hpp"
#include "gui/label.hpp"
#include "gui/mainView.hpp"
#include "gui/scene.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"
#include "gui/window.hpp"

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

    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("LoadSceneIndices");
    
    auto guiScalar = 3.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 240.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar * 0.83f;

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show GUI");

    //auto guiScale = glm::vec3{guiScalar, guiScalar, 0};
    auto guiScaleInv = glm::vec3{1 / guiScalar, 1 / guiScalar, 0};

    glViewport(0, 0, width, height);

    ImguiWrapper::Initialise(window.get());

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0, 0.0f);

    auto spriteManager = Graphics::SpriteManager{};
    auto guiRenderer = Graphics::GuiRenderer{
        width,
        height,
        guiScalar,
        spriteManager};

    auto guiShaderProgram = ShaderProgram{
        "gui.vert.glsl",
        "gui.frag.glsl"};
    auto guiShader = guiShaderProgram.Compile();

    const auto root = static_cast<std::uint8_t>(std::atoi(argv[1]));
    auto currentSceneRef = BAK::HotspotRef{root, *argv[2]};
    
    auto scenes = std::stack<std::unique_ptr<Gui::GDSScene>>{};
    //auto dialog = std::optional<std::string_view>{};

    const auto fontRenderer = Gui::FontRenderer{"GAME.FNT", spriteManager};

    Gui::Window rootWidget{
        spriteManager,
        width / guiScalar,
        height / guiScalar};

    unsigned c = 1;
    auto bt = Gui::ClickButton{
        glm::vec2{20, 20},
        glm::vec2{80, 20},
        fontRenderer,
        "#Change Cursor#",
        [&](){
            rootWidget.GetCursor().PushCursor(c++);
        }};

    auto bt2 = Gui::ClickButton{
        glm::vec2{100, 20},
        glm::vec2{80, 20},
        fontRenderer,
        "Revert Cursor",
        [&](){
            c--;
            rootWidget.GetCursor().PopCursor();
        }};

    Gui::DialogRunner dialogRunner{
        glm::vec2{12, 114},
        glm::vec2{320, 240},
        fontRenderer};

    rootWidget.AddChildBack(&dialogRunner);
    //rootWidget.AddChildFront(&bt);
    //rootWidget.AddChildFront(&bt2);

    const auto MakeScene = [&](auto ref){
        scenes.emplace(
            std::make_unique<Gui::GDSScene>(
                rootWidget.GetCursor(),
                ref,
                spriteManager,
                dialogRunner));
        rootWidget.AddChildFront(scenes.top().get());
        dialogRunner.BeginDialog(scenes.top()->mFlavourText);

        /*for (const auto& hs : scenes.top()->mHotspots.mHotspots)
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
        }*/
    };

    MakeScene(currentSceneRef);

    InputHandler inputHandler{};

    InputHandler::BindMouseToWindow(window.get(), inputHandler);
    InputHandler::BindKeyboardToWindow(window.get(), inputHandler);

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            rootWidget.LeftMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            logger.Debug() << click << "\n";
            rootWidget.LeftMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            logger.Debug() << click << "\n";
        },
        [&](auto click)
        {
            logger.Debug() << click << "\n";
        }
    );

    glm::vec3 mousePos;
    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            rootWidget.MouseMoved(guiScaleInv * pos);
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

        //mDialogScene.SetScene(gdsScene);
        //mDialogScene.SetTitle(dialogTitle);
        //mDialogScene.ShowText(dialogText);
        //mDialogScene.ShowChoices(dialogChoices);

        //rootWidget.AddChildBack(&scene);

        //auto tb = Gui::TextBox{
        //    glm::vec2{16, 120},
        //    glm::vec2{320 - 16 - 16, 240 - 120}};
        //auto text2 = dialog
        //    ? *dialog 
        //    : GetText(scenes.top()->mHotspots.mFlavourText);
        //tb.AddText(fontRenderer, text2);

        //rootWidget.AddChildBack(&tb);
        //rootWidget.AddChildBack(&bt);

        guiRenderer.RenderGui(&rootWidget);

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    glfwTerminate();

    return 0;
}


