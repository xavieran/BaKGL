
add_library(graphics
    IGuiElement.hpp
    glm.hpp
    glfw.hpp glfw.cpp
    line.hpp
    meshObject.hpp meshObject.cpp
    renderer.hpp
    shaderProgram.hpp
    sphere.hpp sphere.cpp
    sprites.hpp
    texture.hpp
    types.hpp
)

target_link_libraries(graphics glfw)
