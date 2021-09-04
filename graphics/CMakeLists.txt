
add_library(graphics
    IGuiElement.hpp IGuiElement.cpp
    glm.hpp
    glfw.hpp glfw.cpp
    guiTypes.hpp guiTypes.cpp
    inputHandler.hpp inputHandler.cpp
    line.hpp
    meshObject.hpp meshObject.cpp
    renderer.hpp renderer.cpp
    guiRenderer.hpp guiRenderer.cpp
    shaderProgram.hpp shaderProgram.cpp
    sphere.hpp sphere.cpp
    sprites.hpp sprites.cpp
    texture.hpp
    types.hpp
)

target_link_libraries(graphics glfw)
