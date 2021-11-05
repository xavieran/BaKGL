set(LINK_3D_LIBRARIES 
    bak
    com
    glfw
    graphics
    imgui
    xbak
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

add_executable (compile_shaders compile_shaders.cpp)

target_link_libraries(compile_shaders
    stdc++
    m
    dl
    pthread 
    ${LINK_3D_LIBRARIES}
    SDL)

list(APPEND SHADERS
    directional
    gui
    shadowMap
)

if(DEFINED ENV{DONT_COMPILE_SHADERS})
else()
    foreach (SHADER IN LISTS SHADERS)
        add_custom_target(
            compileShader${SHADER} ALL COMMAND 
            ${CMAKE_COMMAND} -E env ${LSAN_OPTS} ${CMAKE_BINARY_DIR}/compile_shaders ${SHADER}.vert.glsl ${SHADER}.frag.glsl
            DEPENDS compile_shaders)
    endforeach()
endif()
