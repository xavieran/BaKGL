add_executable (compile_shaders compile_shaders.cpp)

target_link_libraries(compile_shaders
    ${LINK_UNIX_LIBRARIES}
    ${LINK_3D_LIBRARIES}
    com
    graphics
    )

list(APPEND SHADERS
    directional
    instance
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
