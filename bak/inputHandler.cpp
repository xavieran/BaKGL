#include "bak/inputHandler.hpp"

#include <cassert>
#include <functional>

void InputHandler::BindMouseToWindow(GLFWwindow* window, InputHandler& handler)
{
    sHandler = &handler;
    glfwSetMouseButtonCallback(window, InputHandler::MouseAction);
}

void InputHandler::BindKeyboardToWindow(GLFWwindow* window, InputHandler& handler)
{
    sHandler = &handler;
    glfwSetKeyCallback(window, InputHandler::KeyboardAction);
}

void InputHandler::Bind(int key, KeyCallback&& callback)
{
    mKeyBindings.emplace(key, std::move(callback));
}

void InputHandler::BindMouse(int button, MouseCallback&& pressed, MouseCallback&& released)
{
    mMouseBindings.emplace(button,
        std::make_pair(
            std::move(pressed),
            std::move(released)));
}

void InputHandler::HandleInput(GLFWwindow* window)
{
    for (const auto& keyVal : mKeyBindings)
    {
        if (glfwGetKey(window, keyVal.first) == GLFW_PRESS)
        {
            std::invoke(keyVal.second);
        }
    }
}

void InputHandler::HandleMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    const auto it = mMouseBindings.find(button);
    if (it != mMouseBindings.end())
    {
        if (action == GLFW_PRESS)
        {
            double pointerX, pointerY;
            glfwGetCursorPos(window, &pointerX, &pointerY);
            std::invoke(it->second.first, pointerX, pointerY);
        }
        else if (action == GLFW_RELEASE)
        {
            double pointerX, pointerY;
            glfwGetCursorPos(window, &pointerX, &pointerY);
            std::invoke(it->second.second, pointerX, pointerY);
        }
    }
}

void InputHandler::HandleKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const auto it = mKeyBindings.find(key);
    if (it != mKeyBindings.end())
    {
        if (action == GLFW_PRESS)
        {
            std::invoke(it->second);
        }
    }
}

void InputHandler::HandleMouseInput(GLFWwindow* window)
{
    double pointerX, pointerY;
    glfwGetCursorPos(window, &pointerX, &pointerY);

    for (const auto& keyVal : mMouseBindings)
    {
        if (glfwGetMouseButton(window, keyVal.first) == GLFW_PRESS)
            std::invoke(keyVal.second.first, pointerX, pointerY);
        if (glfwGetMouseButton(window, keyVal.first) == GLFW_RELEASE)
            std::invoke(keyVal.second.second, pointerX, pointerY);
    }
}

void InputHandler::MouseAction(GLFWwindow* window, int button, int action, int mods)
{
    assert(sHandler);
    sHandler->HandleMouseCallback(window, button, action, mods);
}

void InputHandler::KeyboardAction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    assert(sHandler);
    sHandler->HandleKeyboardCallback(window, key, scancode, action, mods);
}


InputHandler* InputHandler::sHandler = nullptr;
