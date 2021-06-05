#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <functional>

class InputHandler
{
public:
    using KeyCallback = std::function<void()>;
    using MouseCallback = std::function<void(double, double)>;

    //void BindToWindow(GLFWwindow* window)
    //{
    //    glfwSetMouseButtonCallback(window, this->MouseAction);
    //}

    void Bind(int key, KeyCallback&& callback)
    {
        mKeyBindings.emplace(key, std::move(callback));
    }

    void BindMouse(int button, MouseCallback&& callback)
    {
        mMouseBindings.emplace(button, std::move(callback));
    }


    void HandleInput(GLFWwindow* window)
    {
        for (const auto& keyVal : mKeyBindings)
        {
            if (glfwGetKey(window, keyVal.first) == GLFW_PRESS)
            {
                std::invoke(keyVal.second);
            }
        }
    }

    void HandleMouseInput(GLFWwindow* window)
    {
        for (const auto& keyVal : mMouseBindings)
        {
            if (glfwGetMouseButton(window, keyVal.first) == GLFW_PRESS)
            {
                double pointerX, pointerY;
                glfwGetCursorPos(window, &pointerX, &pointerY);
                std::invoke(keyVal.second, pointerX, pointerY);
            }
        }
    }

    void MouseAction(GLFWwindow* window, int button, int action, int mods)
    {

    }

private:
    std::unordered_map<int, KeyCallback> mKeyBindings;
    std::unordered_map<int, MouseCallback> mMouseBindings;
};
