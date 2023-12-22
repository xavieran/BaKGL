#include "com/logger.hpp"

#include "bak/palette.hpp"
#include "bak/camera.hpp"
#include "bak/worldFactory.hpp"
#include "bak/zone.hpp"

#include "game/systems.hpp"

#include "graphics/glfw.hpp"
#include "graphics/inputHandler.hpp"
#include "graphics/renderer.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/opengl.hpp"
#include "graphics/shaderProgram.hpp"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("display_object");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("Combat");
    Logging::LogState::Disable("CreateFileBuffer");
    Logging::LogState::Disable("LoadFixedObjects");
    Logging::LogState::Disable("MeshObjectStore");
    Logging::LogState::Disable("ZoneItemToMeshObject");
    Logging::LogState::Disable("World");
    Logging::LogState::Disable("LoadEncounter");
    Logging::LogState::Disable("ZoneFactory");
    Logging::LogState::Disable("ShaderProgram");
    Logging::LogState::Disable("GLBuffers");

    if (argc != 3)
    {
        logger.Error() << "Call with <ZONE> <OBJECT>" << std::endl;
        std::exit(1);
    }

    auto objectToDisplay = argv[2];

    auto zoneData = std::make_unique<BAK::Zone>(std::atoi(argv[1]));

    auto guiScalar = 6.0f;

    auto nativeWidth = 320.0f;
    auto nativeHeight = 200.0f;

    auto width = nativeWidth * guiScalar;
    auto height = nativeHeight * guiScalar;

    auto window = Graphics::MakeGlfwWindow(
        height,
        width,
        "BaK");

    auto renderer = Graphics::Renderer{
        width,
        height,
        1024,
        1024};
    renderer.LoadData(zoneData->mObjects, zoneData->mZoneTextures);

    Camera lightCamera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        400 * 30.0f,
        2.0f};
    lightCamera.UseOrthoMatrix(400, 400);

    Camera camera{
        static_cast<unsigned>(width),
        static_cast<unsigned>(height),
        50*30.0f,
        2.0f};
    Camera* cameraPtr = &camera;
    camera.SetPosition({-10, 0, -10});
    Graphics::Light light{
        glm::vec3{.2, -.1, .05},
        glm::vec3{.5, .5, .5},
        glm::vec3{1,.85,.87},
        glm::vec3{.2,.2,.2}
    };

    auto systems = Systems{};
    auto renderable = Renderable{
        systems.GetNextItemId(),
        zoneData->mObjects.GetObject(objectToDisplay),
        {0,0,0},
        {0,0,0},
        glm::vec3{1.0f}};
    systems.AddRenderable(renderable);

    glfwSetCursorPos(window.get(), width/2, height/2);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    Graphics::InputHandler inputHandler{};
    inputHandler.Bind(GLFW_KEY_W, [&]{ cameraPtr->MoveForward(); });
    inputHandler.Bind(GLFW_KEY_S, [&]{ cameraPtr->MoveBackward(); });
    inputHandler.Bind(GLFW_KEY_A, [&]{ cameraPtr->StrafeLeft(); });
    inputHandler.Bind(GLFW_KEY_D, [&]{ cameraPtr->StrafeRight(); });
    inputHandler.Bind(GLFW_KEY_X, [&]{ cameraPtr->RotateVerticalUp(); });
    inputHandler.Bind(GLFW_KEY_Y, [&]{ cameraPtr->RotateVerticalDown(); });
    inputHandler.Bind(GLFW_KEY_Q, [&]{ cameraPtr->RotateLeft(); });
    inputHandler.Bind(GLFW_KEY_E, [&]{ cameraPtr->RotateRight(); });
    Graphics::InputHandler::BindKeyboardToWindow(window.get(), inputHandler);
    Graphics::InputHandler::BindMouseToWindow(window.get(), inputHandler);

    double currentTime;
    double lastTime = 0;
    float deltaTime;

    do
    {
        currentTime = glfwGetTime();

        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        cameraPtr->SetDeltaTime(deltaTime);

        inputHandler.HandleInput(window.get());
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        // Dark blue background
        glClearColor(0.15f, 0.31f, 0.36f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.DrawWithShadow(
            systems.GetRenderables(),
            light,
            lightCamera,
            *cameraPtr);

        // Swap buffers
        glfwSwapBuffers(window.get());
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE ) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0 );
    
    return 0;
}
