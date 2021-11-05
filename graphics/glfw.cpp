#include "graphics/glfw.hpp"

#include "com/logger.hpp"
#include <stdexcept>

namespace Graphics {

void GLAPIENTRY OpenGlMessageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    const auto GetLogLevel = [](GLenum level)
    {
        if (level == GL_DEBUG_SEVERITY_NOTIFICATION)
            return Logging::LogLevel::Spam;
        else if (level == GL_DEBUG_SEVERITY_LOW)
            return Logging::LogLevel::Debug;
        else if (level == GL_DEBUG_SEVERITY_MEDIUM)
            return Logging::LogLevel::Info;
        else
            return Logging::LogLevel::Fatal;
    };
    const auto logLevel = GetLogLevel(severity);
    static const auto& logger = Logging::LogState::GetLogger("OpenGL");
    logger.Log(logLevel) << " type = 0x" << std::hex 
        << type << " severity = 0x" << severity << std::dec 
        << " message = " << message << std::endl;

    //if (type == GL_DEBUG_TYPE_ERROR)
    //    throw std::runtime_error("OpenGL Error");
}

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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGlMessageCallback, 0);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    return std::unique_ptr<GLFWwindow, DestroyGlfwWindow>{window};
}

}
