
add_library(graphics
    glfw.hpp glfw.cpp
    line.hpp
    meshObject.hpp
    renderer.hpp
    shaderProgram.hpp
    sphere.hpp sphere.cpp
    texture.hpp
)

target_link_libraries(graphics glfw)
