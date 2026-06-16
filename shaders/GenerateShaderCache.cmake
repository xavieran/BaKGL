function(make_includable input_file output_file)
    file(READ ${input_file} content)
    set(delim "for_c++_include")
    set(content "R\"${delim}(\n${content})${delim}\"")
    file(WRITE ${output_file} "${content}")
endfunction(make_includable)

file(GLOB SHADER_FILES ${SHADER_SRC_DIR}/*.glsl)

set(SHADER_BINARY_CACHE "")
set(SEPARATOR "\n")

foreach (SHADER_PATH IN LISTS SHADER_FILES)
    get_filename_component(SHADER_FILE "${SHADER_PATH}" NAME)
    make_includable(${SHADER_PATH} ${OUTPUT_DIR}/gen/${SHADER_FILE})
    set(SHADER_BINARY_CACHE "${SHADER_BINARY_CACHE}${SEPARATOR}{\"${SHADER_FILE}\", \n#include \"shaders/gen/${SHADER_FILE}\"\n}")
    set(SEPARATOR ",\n")
endforeach()

file(WRITE "${OUTPUT_DIR}/shader_binary_cache.hxx" "${SHADER_BINARY_CACHE}")
