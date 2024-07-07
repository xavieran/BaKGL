add_executable (compile_shaders compile_shaders.cpp)

add_library(shaders shaders.hpp shaders.cpp)

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

function(make_includable input_file output_file)
    file(READ ${input_file} content)
    set(delim "for_c++_include")
    set(content "R\"${delim}(\n${content})${delim}\"")
    file(WRITE ${output_file} "${content}")
endfunction(make_includable)

file(GLOB SHADER_FILES *.glsl)

set(SHADER_BINARY_CACHE "")
set(SEPARATOR "\n")

foreach (SHADER_PATH IN LISTS SHADER_FILES)
    get_filename_component(SHADER_FILE "${SHADER_PATH}" NAME)
    make_includable(${SHADER_PATH} gen/${SHADER_FILE})
    set(SHADER_BINARY_CACHE "${SHADER_BINARY_CACHE}${SEPARATOR}{\"${SHADER_FILE}\", \n#include \"shaders/gen/${SHADER_FILE}\"\n}")
    set(SEPARATOR ",\n")
endforeach()

file(WRITE "shader_binary_cache.hxx" "${SHADER_BINARY_CACHE}")
