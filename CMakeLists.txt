cmake_minimum_required(VERSION 3.19)
project( BaKGL )

# Project wide CXX flags
set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-private-field -Wno-format-security -Wno-c++17-extensions")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror ${CXX_IGNORE}")
set(CMAKE_CXX_STANDARD 20)


# Packages
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/glm")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# My project directories
include_directories(".")

add_subdirectory("app")
add_subdirectory("graphics")
add_subdirectory("imgui")
add_subdirectory("src")
add_subdirectory("shaders")
add_subdirectory("xbak")
