#include "graphics/inputHandler.hpp"

#include "com/assert.hpp"

#include <functional>

namespace Graphics {

InputHandler::InputHandler() noexcept
:
    mHandleInput{true},
    mKeyBindings{},
    mCharacterCallback{},
    mMouseBindings{},
    mMouseMovedBinding{},
    mMouseScrolledBinding{}
{}

void InputHandler::BindMouseToWindow(GLFWwindow* window, InputHandler& handler)
{
    sHandler = &handler;
    glfwSetMouseButtonCallback(window, InputHandler::MouseAction);
    glfwSetCursorPosCallback(window, InputHandler::MouseMotionAction);
    glfwSetScrollCallback(window, InputHandler::MouseScrollAction);
}

void InputHandler::BindKeyboardToWindow(GLFWwindow* window, InputHandler& handler)
{
    sHandler = &handler;
    glfwSetCharCallback(window, InputHandler::CharacterAction);
    //glfwSetKeyCallback(window, InputHandler::KeyboardAction);
}

void InputHandler::Bind(int key, KeyCallback&& callback)
{
    mKeyBindings.emplace(key, std::move(callback));
}

void InputHandler::BindCharacter(CharacterCallback&& callback)
{
    mCharacterCallback = std::move(callback);
}

void InputHandler::BindMouse(int button, MouseCallback&& pressed, MouseCallback&& released)
{
    mMouseBindings.emplace(button,
        std::make_pair(
            std::move(pressed),
            std::move(released)));
}

void InputHandler::BindMouseMotion(MouseCallback&& moved)
{
    mMouseMovedBinding = std::move(moved);
}

void InputHandler::BindMouseScroll(MouseCallback&& scrolled)
{
    mMouseScrolledBinding = std::move(scrolled);
}

void InputHandler::HandleInput(GLFWwindow* window)
{
    if (mHandleInput)
    {
        for (const auto& keyVal : mKeyBindings)
        {
            if (glfwGetKey(window, keyVal.first) == GLFW_PRESS)
            {
                std::invoke(keyVal.second);
            }
        }
    }
}

void InputHandler::HandleMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (mHandleInput)
    {
        const auto it = mMouseBindings.find(button);
        if (it != mMouseBindings.end())
        {
            if (action == GLFW_PRESS)
            {
                double pointerX, pointerY;
                glfwGetCursorPos(window, &pointerX, &pointerY);
                std::invoke(it->second.first, glm::vec2{pointerX, pointerY});
            }
            else if (action == GLFW_RELEASE)
            {
                double pointerX, pointerY;
                glfwGetCursorPos(window, &pointerX, &pointerY);
                std::invoke(it->second.second, glm::vec2{pointerX, pointerY});
            }
        }
    }
}

void InputHandler::HandleMouseMotionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mMouseMovedBinding)
        std::invoke(mMouseMovedBinding, glm::vec2{xpos, ypos});
}

void InputHandler::HandleMouseScrollCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mMouseScrolledBinding)
        std::invoke(mMouseScrolledBinding, glm::vec2{xpos, ypos});
}

void InputHandler::HandleKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (mHandleInput)
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
}

void InputHandler::HandleCharacterCallback(GLFWwindow* window, unsigned character)
{
    if (mHandleInput)
    {
        if (mCharacterCallback)
        {
            mCharacterCallback(character & 0xff);
        }
    }
}

void InputHandler::HandleMouseInput(GLFWwindow* window)
{
    
    if (mHandleInput)
    {
        double pointerX, pointerY;
        glfwGetCursorPos(window, &pointerX, &pointerY);

        for (const auto& keyVal : mMouseBindings)
        {
            if (glfwGetMouseButton(window, keyVal.first) == GLFW_PRESS)
                std::invoke(keyVal.second.first, glm::vec2{pointerX, pointerY});
            if (glfwGetMouseButton(window, keyVal.first) == GLFW_RELEASE)
                std::invoke(keyVal.second.second, glm::vec2{pointerX, pointerY});
        }
    }
}

void InputHandler::MouseAction(GLFWwindow* window, int button, int action, int mods)
{
    ASSERT(sHandler);
    sHandler->HandleMouseCallback(window, button, action, mods);
}

void InputHandler::MouseMotionAction(GLFWwindow* window, double xpos, double ypos)
{
    ASSERT(sHandler);
    sHandler->HandleMouseMotionCallback(window, xpos, ypos);
}

void InputHandler::MouseScrollAction(GLFWwindow* window, double xpos, double ypos)
{
    ASSERT(sHandler);
    sHandler->HandleMouseScrollCallback(window, xpos, ypos);
}

void InputHandler::KeyboardAction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ASSERT(sHandler);
    sHandler->HandleKeyboardCallback(window, key, scancode, action, mods);
}

void InputHandler::CharacterAction(GLFWwindow* window, unsigned character)
{
    ASSERT(sHandler);
    sHandler->HandleCharacterCallback(window, character);
}


InputHandler* InputHandler::sHandler = nullptr;

}
