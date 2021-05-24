#include_directories("src")
# Be nice to run this over all the shaders
# Need to add dependencies on "compile_shaders" and on the shaders referenced...
execute_process(
	COMMAND "${CMAKE_BINARY_DIR}/compile_shaders vertex.glsl fragment.glsl" 
	RESULT_VARIABLE STATUS)

if(${STATUS} AND NOT ${STATUS} EQUAL "0")
    message(FATAL_ERROR "Failed to compile shaders")
endif()


