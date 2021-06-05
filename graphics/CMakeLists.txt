
add_library(graphics
    glm.hpp
    glfw.hpp glfw.cpp
    line.hpp
    meshObject.hpp meshObject.cpp
    renderer.hpp
    shaderProgram.hpp
    sphere.hpp sphere.cpp
    texture.hpp
)

target_link_libraries(graphics glfw)
