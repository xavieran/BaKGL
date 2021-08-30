#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "bak/font.hpp"
#include "bak/gameState.hpp"
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
#include "graphics/inputHandler.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/renderer.hpp"
#include "graphics/shaderProgram.hpp"
#include "graphics/texture.hpp"

#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/clickButton.hpp"
#include "gui/contents.hpp"
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

    //Logging::LogState::Disable("LoadScenes");
    //Logging::LogState::Disable("LoadSceneIndices");
    Logging::LogState::Disable("Gui::Actors");
    
    auto guiScalar = 2.0f;

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

    const auto font = Gui::Font{"GAME.FNT", spriteManager};
    const auto backgrounds = Gui::Backgrounds{spriteManager};


    Gui::Window rootWidget{
        spriteManager,
        width / guiScalar,
        height / guiScalar};



    auto gs = BAK::GameState{};
    Gui::DialogRunner dialogRunner{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        backgrounds,
        font,
        gs};

    //auto contents = Gui::GenericRequestScreen{
    //    spriteManager,
    //    "FULLMAP.PAL",
    //    "REQ_FMAP.DAT",
    //    "FULLMAP.SCX"
    //};
    //rootWidget.AddChildFront(&contents);

    rootWidget.AddChildFront(&dialogRunner);

    scenes.emplace(
        std::make_unique<Gui::GDSScene>(
            rootWidget.GetCursor(),
            currentSceneRef,
            spriteManager,
            dialogRunner));

    //auto act = Gui::Actors{spriteManager};
    //const auto [ss, im] = act.GetActor(1);
    //auto sumani = Gui::Widget{
    //    Graphics::DrawMode::Sprite,
    //    ss,
    //    im,
    //    Graphics::ColorMode::Texture,
    //    Gui::Color::debug,
    //    glm::vec2{100,19},
    //    spriteManager.GetSpriteSheet(ss).GetDimensions(im),
    //    true};
    //rootWidget.AddChildFront(&sumani);
    rootWidget.AddChildFront(scenes.top().get());
    dialogRunner.BeginDialog(scenes.top()->mFlavourText);
    //dialogRunner.BeginDialog(BAK::KeyTarget{0x2dc6d4});
    //dialogRunner.BeginDialog(BAK::KeyTarget{0x2dc6ed});
    //dialogRunner.BeginDialog(BAK::OffsetTarget{30, 0x1793});

    // Set up input callbacks
    Graphics::InputHandler inputHandler{};

    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_LEFT,
        [&](auto click)
        {
            rootWidget.LeftMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            rootWidget.LeftMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouse(
        GLFW_MOUSE_BUTTON_RIGHT,
        [&](auto click)
        {
            rootWidget.RightMousePress(guiScaleInv * click);
        },
        [&](auto click)
        {
            rootWidget.RightMouseRelease(guiScaleInv * click);
        }
    );

    inputHandler.BindMouseMotion(
        [&](auto pos)
        {
            rootWidget.MouseMoved(guiScaleInv * pos);
        }
    );

    // Graphics stuff
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    guiShader.UseProgramGL();

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

    return 0;
}
