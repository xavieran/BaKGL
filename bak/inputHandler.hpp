#pragma once


#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <cassert>
#include <functional>


class InputHandler
{
public:
    using KeyCallback = std::function<void()>;
    using MouseCallback = std::function<void(glm::vec3)>;

    static void BindMouseToWindow(GLFWwindow* window, InputHandler& handler);
    static void BindKeyboardToWindow(GLFWwindow* window, InputHandler& handler);

    void Bind(int key, KeyCallback&& callback);
    void BindMouse(
        int button,
        MouseCallback&& pressed,
        MouseCallback&& released);
    void BindMouseMotion(MouseCallback&& moved);


    void HandleInput(GLFWwindow* window);
    void HandleMouseInput(GLFWwindow* window);
    void HandleKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void HandleMouseCallback(GLFWwindow* window, int button, int action, int mods);
    void HandleMouseMotionCallback(GLFWwindow* window, double xpos, double ypos);


private:
    static void MouseAction(GLFWwindow* window, int button, int action, int mods);
    static void MouseMotionAction(GLFWwindow* window, double xpos, double ypos);
    static void KeyboardAction(GLFWwindow* window, int key, int scancode, int action, int mods);

    static InputHandler* sHandler;

    std::unordered_map<int, KeyCallback> mKeyBindings;
    std::unordered_map<int, std::pair<MouseCallback, MouseCallback>> mMouseBindings;
    MouseCallback mMouseMovedBinding;
};
