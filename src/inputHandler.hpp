#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <functional>

class InputHandler
{
public:
    using Callback = std::function<void()>;

    void Bind(int key, Callback&& callback)
    {
        mBindings.emplace(key, std::move(callback));
    }

    void HandleInput(GLFWwindow* window)
    {
        for (const auto& keyVal : mBindings)
        {
            if (glfwGetKey(window, keyVal.first) == GLFW_PRESS)
            {
                std::invoke(keyVal.second);
            }
        }
    }

private:
    std::unordered_map<int, std::function<void()>> mBindings;
};
