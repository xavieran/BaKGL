cmake_minimum_required(VERSION 3.19)
project( BaKGL )

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include_directories(".")

add_subdirectory("graphics")
add_subdirectory("imgui")
add_subdirectory("xbak")

include_directories("src")

set(APP_INCLUDES
    graphics
    imgui
    xbak
)

set(LINK_3D_LIBRARIES 
    glfw
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-g -std=c++20 -Wall -Wextra -Werror ${CXX_IGNORE}")

list(APPEND APP_BINARIES
    compile_shaders 
    dialog_explorer
    display_dialog
    display_gds
    display_object
    display_tile
    display_save
    main2d
    main3d
    show_imgui
)

foreach (APP_BIN IN LISTS APP_BINARIES)
    add_executable (${APP_BIN} app/${APP_BIN}.cpp ${APP_INCLUDES})
    target_link_libraries(${APP_BIN} dl pthread  ${LINK_3D_LIBRARIES} SDL ${APP_INCLUDES})
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
    target_link_libraries(${XBAK_BIN} dl pthread xbak SDL)
endforeach()

add_executable (xbak_main xbak/xbak.cc ${XBAK} ${SDL_INCLUDE_DIR})
target_link_libraries(xbak_main dl pthread xbak SDL)

