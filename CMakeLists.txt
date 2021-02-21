cmake_minimum_required(VERSION 3.19)
project( BaKSandbox )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/boost)

set(BOOST_REQUESTED_VERSION 1.72.0)
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)

#add_subdirectory(glad)

add_subdirectory("imgui")
include_directories("imgui")

add_subdirectory("xbak")
include_directories("xbak")

include_directories("src")
include_directories("shaders")

set(BAK_INCLUDES
    xbak
    imgui
)

set(LINK_3D_LIBRARIES 
    glfw
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-O2 -std=c++20 -Wall -Wextra -Werror ${CXX_IGNORE}")

list(APPEND BAK_BINARIES
    compile_shaders 
    dialog_explorer
    display_dialog
    display_object
    display_tile
    display_save
    show_imgui
    main2d
    main3d
)

foreach (BAK_BIN IN LISTS BAK_BINARIES)
    add_executable (${BAK_BIN} src/${BAK_BIN}.cpp ${BAK_INCLUDES})
    target_link_libraries(${BAK_BIN} dl pthread ${Boost_LIBRARIES} ${LINK_3D_LIBRARIES} imgui SDL xbak)
endforeach()

# Be nice to run this over all the shaders
# Need to add dependencies on "compile_shaders" and on the shaders referenced...
execute_process(
	COMMAND "${CMAKE_BINARY_DIR}/compile_shaders vertex.glsl fragment.glsl" 
	RESULT_VARIABLE STATUS)

if(${STATUS} AND NOT ${STATUS} EQUAL "0")
    message(FATAL_ERROR "Failed to compile shaders")
endif()

# XBaK Executables
list(APPEND XBAK_BINARIES
    dumpbmx
    dumpact_bmx
    dumpchar_bmx
    dumpzone_bmx
    dumppal
    dumpobj
    dumptbl
    dumpscx
    dumpwld
    resourcedemo
)

foreach (XBAK_BIN IN LISTS XBAK_BINARIES)
    add_executable (${XBAK_BIN} xbak/${XBAK_BIN}.cc ${XBAK} ${SDL_INCLUDE_DIR})
    target_link_libraries(${XBAK_BIN} dl pthread ${Boost_LIBRARIES} xbak SDL)
endforeach()

add_executable (xbak_main xbak/xbak.cc ${XBAK} ${SDL_INCLUDE_DIR})
target_link_libraries(xbak_main dl pthread ${Boost_LIBRARIES} xbak SDL)

