#include "com/logger.hpp"

#include "graphics/glfw.hpp"
#include "graphics/guiRenderer.hpp"
#include "graphics/inputHandler.hpp"

#include "gui/actors.hpp"
#include "gui/animatorStore.hpp"
#include "gui/backgrounds.hpp"
#include "gui/fontManager.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/dynamicTTM.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <memory>

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

    if (argc < 2)
    {
        std::cerr << "Usage: "
            << argv[0] << " BASENAME\n";
        return -1;
    }
    
    std::string basename{argv[1]};
    
    const auto font = Gui::Font{"GAME.FNT", spriteManager};
    const auto bookFont = Gui::Font{"BOOK.FNT", spriteManager};
    const auto actors = Gui::Actors{spriteManager};
    const auto backgrounds = Gui::Backgrounds{spriteManager};

    Gui::Window rootWidget{
        spriteManager,
        width / guiScalar,
        height / guiScalar};

    Gui::AnimatorStore animatorStore{};

    auto dynamicTTM = Gui::DynamicTTM(
        spriteManager,
        animatorStore,
        font,
        backgrounds,
        [&](){ },
        [&](auto book){ });

    rootWidget.AddChildBack(dynamicTTM.GetScene());
    dynamicTTM.BeginScene(basename + ".ADS", basename + ".TTM");

    bool playing = false;

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
            dynamicTTM.AdvanceAction();
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
    
    double currentTime = 0;
    double lastTime = 0;
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentTime = glfwGetTime();
        animatorStore.OnTimeDelta(currentTime - lastTime);
        lastTime = currentTime;

        guiRenderer.RenderGui(&rootWidget);

        glfwSwapBuffers(window.get());

        glfwPollEvents();
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    return 0;
}
