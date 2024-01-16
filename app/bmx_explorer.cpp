#include "com/logger.hpp"

#include "graphics/inputHandler.hpp"
#include "graphics/glm.hpp"
#include "graphics/glfw.hpp"
#include "graphics/guiRenderer.hpp"

#include "gui/icons.hpp"
#include "gui/textBox.hpp"
#include "gui/window.hpp"

#include "imgui/imguiWrapper.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <memory>
#include <sstream>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    auto guiScalar = 4.0f;

    auto width = 1600.0f;
    auto height = 1000.0f;
    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BMX Explorer");

    glViewport(0, 0, width, height);

    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    auto spriteManager = Graphics::SpriteManager{};
    auto guiRenderer = Graphics::GuiRenderer{
        width,
        height,
        guiScalar,
        spriteManager};

    auto fontManager = Gui::FontManager{spriteManager};

    auto root = Gui::Window{
        spriteManager,
        width / guiScalar,
        height / guiScalar};

    auto icons = Gui::Icons{spriteManager};
    unsigned iconI = 0;
    auto picture = Gui::Widget{
        Gui::ImageTag{},
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        glm::vec2{50,50},
        glm::vec2{100,100},
        true};

    root.AddChildBack(&picture);
    root.HideCursor();

    const auto& font = fontManager.GetSpellFont();
    std::vector<std::unique_ptr<Gui::Widget>> spellIcons{};
    auto pos = glm::vec2{2, 100};
    for (unsigned i = 0; i < 96; i++)
    {
        if (i == 24)
        {
            pos = glm::vec2{2, 120};
        }
        spellIcons.emplace_back(
            std::make_unique<Gui::Widget>(
                Graphics::DrawMode::Sprite,
                font.GetSpriteSheet(),
                static_cast<Graphics::TextureIndex>(
                    font.GetFont().GetIndex(i)),
                Graphics::ColorMode::Texture,
                glm::vec4{1.f, 0.f, 0.f, 1.f},
                pos,
                glm::vec2{font.GetFont().GetWidth(i), font.GetFont().GetHeight()},
                true
                ));
        pos += glm::vec2{font.GetFont().GetWidth(i) + 1, 0};
        root.AddChildBack(spellIcons.back().get());
    }

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_RIGHT, [&]{
        iconI++;
        const auto& [ss, ti, dims] = icons.GetInventoryLockIcon(iconI);
        picture.SetSpriteSheet(ss);
        picture.SetTexture(ti);
        picture.SetDimensions(dims);
        logger.Debug() << "Pic: " << picture << "\n";
    });
    inputHandler.Bind(GLFW_KEY_LEFT, [&]{
        iconI--;
        const auto& [ss, ti, dims] = icons.GetInventoryIcon(iconI);
        picture.SetSpriteSheet(ss);
        picture.SetTexture(ti);
        picture.SetDimensions(dims);
        logger.Debug() << "Pic: " << picture << "\n";
    });
    inputHandler.BindMouse(GLFW_MOUSE_BUTTON_LEFT, [&](auto p)
    {
        logger.Debug() << p << "\n";
    },
    [](auto){});

    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);

    double currentTime = 0;
    double lastTime = 0;
    float deltaTime = 0;

    glfwSetCursorPos(window.get(), width / 2, height / 2);

    glEnable(GL_MULTISAMPLE);  

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    double pointerPosX, pointerPosY;

    double acc = 0;
    unsigned i = 0;
    do
    {
        inputHandler.SetHandleInput(true);

        glfwPollEvents();
        glfwGetCursorPos(window.get(), &pointerPosX, &pointerPosY);
        inputHandler.HandleInput(window.get());

        // { *** Draw 2D GUI ***
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        guiRenderer.RenderGui(&root);

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    glfwTerminate();

    return 0;
}


