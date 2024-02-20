#include "bak/encounter/teleport.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include "graphics/glfw.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/inputHandler.hpp"
#include "graphics/texture.hpp"

#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/guiManager.hpp"
#include "gui/staticTTM.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <sstream>

// SDL...
#undef main

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("LoadSceneIndices");
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("Gui::Actors");
    Logging::LogState::Disable("CreateFileBuffer");
    
    auto guiScalar = 3.5f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 240.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar * 0.83f;

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "Show GUI");

    //auto guiScale = glm::vec3{guiScalar, guiScalar, 0};
    auto guiScaleInv = glm::vec2{1 / guiScalar, 1 / guiScalar};

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

    if (argc < 3)
    {
        std::cerr << "Usage: "
            << argv[0] << " ADS TTM\n";
        return -1;
    }
    
    std::string adsFile{argv[1]};
    std::string ttmFile{argv[2]};
    auto gameState = BAK::GameState{};
    
    const auto font = Gui::Font{"GAME.FNT", spriteManager};
    const auto actors = Gui::Actors{spriteManager};
    const auto backgrounds = Gui::Backgrounds{spriteManager};

    Gui::Window rootWidget{
        spriteManager,
        width / guiScalar,
        height / guiScalar};


    auto fb1 = BAK::FileBufferFactory::Get().CreateDataBuffer(adsFile);
    auto indices = BAK::LoadSceneIndices(fb1);
    auto fb2 = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    auto scenes = BAK::LoadScenes(fb2);

    logger.Info() << "Playing: " <<adsFile << " " << ttmFile << "\n";
    logger.Info() << "ADS: " << "\n";
    for (auto& [key, ads] : indices)
    {
        logger.Info() << "Key: " << key << " ADS: " << ads << "\n";
    }
    logger.Info() << "TTM: " << "\n";
    for (auto& [key, scene] : scenes)
    {
        logger.Info() << "Key: " << key << " TTM: " << scene << "\n";
    }

    auto staticTtm = Gui::StaticTTM{
        spriteManager,
        scenes[1],
        scenes[1]
    };

    rootWidget.AddChildBack(staticTtm.GetScene());

    // Set up input callbacks
    Graphics::InputHandler inputHandler{};

    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](const auto click)
        {
            rootWidget.OnMouseEvent(
                Gui::LeftMousePress{guiScaleInv * click});
        },
        [&](const auto click)
        {
            rootWidget.OnMouseEvent(
                Gui::LeftMouseRelease{guiScaleInv * click});
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            rootWidget.OnMouseEvent(
                Gui::RightMousePress{guiScaleInv * click});
        },
        [&](auto click)
        {
            rootWidget.OnMouseEvent(
                Gui::RightMouseRelease{guiScaleInv * click});
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            rootWidget.OnMouseEvent(
                Gui::MouseMove{guiScaleInv * pos});
        }
    );

    // Graphics stuff
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        guiRenderer.RenderGui(&rootWidget);

        glfwSwapBuffers(window.get());

        glfwPollEvents();
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    delete gameState.GetGameData();

    return 0;
}
