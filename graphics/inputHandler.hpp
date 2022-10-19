#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <functional>

namespace Graphics {

class InputHandler
{
public:
    using KeyCallback = std::function<void()>;
    using MouseCallback = std::function<void(glm::vec2)>;

    InputHandler() noexcept;

    static void BindMouseToWindow(GLFWwindow* window, InputHandler& handler);
    static void BindKeyboardToWindow(GLFWwindow* window, InputHandler& handler);

    void SetHandleInput(bool value)
    {
        mHandleInput = value;
    }

    void Bind(int key, KeyCallback&& callback);
    void BindMouse(
        int button,
        MouseCallback&& pressed,
        MouseCallback&& released);
    void BindMouseMotion(MouseCallback&& moved);
    void BindMouseScroll(MouseCallback&& scrolled);

    void HandleInput(GLFWwindow* window);
    void HandleMouseInput(GLFWwindow* window);
    void HandleKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void HandleMouseCallback(GLFWwindow* window, int button, int action, int mods);
    void HandleMouseMotionCallback(GLFWwindow* window, double xpos, double ypos);
    void HandleMouseScrollCallback(GLFWwindow* window, double xpos, double ypos);

private:
    static void MouseAction(GLFWwindow* window, int button, int action, int mods);
    static void MouseMotionAction(GLFWwindow* window, double xpos, double ypos);
    static void MouseScrollAction(GLFWwindow* window, double xpos, double ypos);
    static void KeyboardAction(GLFWwindow* window, int key, int scancode, int action, int mods);

    static InputHandler* sHandler;

    bool mHandleInput;

    std::unordered_map<int, KeyCallback> mKeyBindings;
    std::unordered_map<int, std::pair<MouseCallback, MouseCallback>> mMouseBindings;
    MouseCallback mMouseMovedBinding;
    MouseCallback mMouseScrolledBinding;
};

}
