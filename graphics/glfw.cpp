#include "graphics/glfw.hpp"

#include "com/logger.hpp"

namespace Graphics {

std::unique_ptr<GLFWwindow, DestroyGlfwWindow> MakeGlfwWindow(
    unsigned height,
    unsigned width,
    std::string_view title)
{
    const auto logger = Logging::LogState::GetLogger("GLFW");
    glfwSetErrorCallback([](int error, const char* desc){ puts(desc); });

    if( !glfwInit() )
    {
        logger.Error() << "Failed to initialize GLFW" << std::endl;
        throw std::runtime_error("Failed to initialize GLFW");
    }

    GLFWwindow* window{nullptr};

    constexpr auto antiAliasingSamples = 4;
    glfwWindowHint(GLFW_SAMPLES, antiAliasingSamples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if( window == nullptr)
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window");
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW");
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    return std::unique_ptr<GLFWwindow, DestroyGlfwWindow>{window};
}

}
