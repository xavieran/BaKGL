#include "com/logger.hpp"

#include "graphics/shaderProgram.hpp"

#include <GLFW/glfw3.h>

void ErrorCallback(int code, const char* desc)
{
    const auto& logger = Logging::LogState::GetLogger("GLFW");
    logger.Error() << " Code: " << code << " desc: " << desc << std::endl;
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    if (argc != 3)
    {
        logger.Error() << "Usage: compile_shaders vertex fragment";
        return -1;
    }

    const auto vertexShaderPath = std::string{argv[1]};
    const auto fragmentShaderPath = std::string{argv[2]};

    auto program = ShaderProgram{vertexShaderPath, std::optional<std::string>{}, fragmentShaderPath};

    logger.Info() << "Compiling " << vertexShaderPath << " and " << fragmentShaderPath << std::endl;

    glfwSetErrorCallback(ErrorCallback);

    if( !glfwInit() )
    {
        logger.Error() << "Failed to open GLFW window" << std::endl;
        return -1;
    }

    GLFWwindow* window;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    unsigned height = 600;
    unsigned width  = 800;

    window = glfwCreateWindow( width, height, "BaK", NULL, NULL);
    if( window == NULL )
    {
        logger.Error() << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        logger.Error() << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    

    try
    {
        auto programID = program.Compile();
    }
    catch (const std::exception& e)
    {
        logger.Error() << "Failed to compile shaders: " << e.what() << std::endl;
        return -1;
    }

    logger.Info() << "Successfully compiled shaders" << std::endl;

    return 0;
}

