add_executable (compile_shaders compile_shaders.cpp)

set(SHADER_CACHE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/shader_binary_cache.hxx)

add_library(shaders shaders.hpp shaders.cpp ${SHADER_CACHE_HEADER})

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
    pick
    pick.sprite
    sprite
    shadowMap
)

if(DEFINED ENV{DONT_COMPILE_SHADERS})
else()
    foreach (SHADER IN LISTS SHADERS)
        add_custom_target(
            compileShader${SHADER} ALL COMMAND 
            ${CMAKE_COMMAND} -E env ${LSAN_OPTS} ${CMAKE_BINARY_DIR}/compile_shaders ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}.vert.glsl ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}.frag.glsl
            DEPENDS compile_shaders)
    endforeach()
endif()

file(GLOB SHADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl)

add_custom_command(
    OUTPUT ${SHADER_CACHE_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -DSHADER_SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DOUTPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateShaderCache.cmake
    DEPENDS ${SHADER_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/GenerateShaderCache.cmake
    VERBATIM)
