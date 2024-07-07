enable_testing()

add_library(graphics
    IGuiElement.hpp IGuiElement.cpp
    glm.hpp
    glfw.hpp glfw.cpp
    guiTypes.hpp guiTypes.cpp
    framebuffer.hpp framebuffer.cpp
    inputHandler.hpp inputHandler.cpp
    line.hpp
    meshObject.hpp meshObject.cpp
    opengl.hpp opengl.cpp
    guiRenderer.hpp guiRenderer.cpp
    shaderProgram.hpp shaderProgram.cpp
    sphere.hpp sphere.cpp
    sprites.hpp sprites.cpp
    texture.hpp
    types.hpp
)

target_link_libraries(
    graphics
    com
    shaders
    ${LINK_3D_LIBRARIES})
