cmake_minimum_required(VERSION 3.19)
project( BaKGL )

# Project wide CXX flags
string(CONCAT CXX_IGNORES
    "-Wno-unused-variable "
    "-Wno-unused-parameter "
#   "-Wno-unused-but-set-parameter "
#   " -Wno-c++17-extensions "
    "-Wno-unused-private-field "
    "-Wno-unused-function "
    "-Wno-format-security "
    "-Wno-deprecated-enum-enum-conversion "
    )
set(CXX_IGNORE ${CXX_IGNORES})

set(CXX_ASAN "-fsanitize=address -fno-omit-frame-pointer")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror ${CXX_ASAN} ${CXX_IGNORE}")
set(CMAKE_CXX_STANDARD 20)

set(LSAN_OPTS "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/.lsan.supp")

#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*,-llvmlibc-*,-modernize-use-trailing-return-type")

# I just want all the bins in the build dir
set(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR})

# Packages
find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_sound REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDL_mixer_INCLUDE_DIR})
include_directories(${SDL_sound_INCLUDE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/glm")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

## Now simply link against gtest or gtest_main as needed. Eg
#add_executable(example example.cpp)
#target_link_libraries(example gtest_main)
#add_test(NAME example_test COMMAND example)

# My project directories
include_directories(".")

add_subdirectory("app")
add_subdirectory("bak")
add_subdirectory("com")
add_subdirectory("graphics")
add_subdirectory("gui")
add_subdirectory("imgui")
add_subdirectory("shaders")
add_subdirectory("xbak")
