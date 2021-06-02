#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>

namespace Graphics {

struct DestroyGlfwWindow
{
    void operator()(GLFWwindow* window)
    {
        glfwDestroyWindow(window);
    }
};

std::unique_ptr<GLFWwindow, DestroyGlfwWindow> MakeGlfwWindow(
    unsigned height,
    unsigned width,
    std::string_view title);

}
