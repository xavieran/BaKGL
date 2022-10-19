cmake_minimum_required(VERSION 3.19)
message ("CMAKE VERSION: " "${CMAKE_VERSION}")

project( BaKGL )

# Project wide CXX flags
string(CONCAT CXX_IGNORES
    "-Wno-unused-variable "
    "-Wno-unused-parameter "
#   "-Wno-unused-but-set-parameter "
#   "-Wno-c++17-extensions "
    "-Wno-unused-private-field "
    "-Wno-unused-function "
    "-Wno-format-security "
    "-Wno-deprecated-enum-enum-conversion "
    )

#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    set(CXX_IGNORE "/wd4244 /wd4267 /wd4996 /wd4305 /wd4065 /wd4146 /wd4101 /wd4312 /wd4311 /wd4302")
    set(CXX_FLAGS "/W3 /WX /EHsc")
    
    set(CXX_USAN "")
    set(CXX_ASAN "/fsanitize=address")
else()
    set(CXX_IGNORE ${CXX_IGNORES})
    set(CXX_FLAGS "-Wall -Wextra -Werror")

    # FileBuffer violates alignment pretty regularly. Will need to fix at some point
    #set(CXX_USAN "-fvisibility=hidden -fsanitize=undefined -fno-sanitize=vptr -fno-sanitize=alignment")
    set(CXX_ASAN "-fsanitize=address -fno-omit-frame-pointer")
    
    set(CXX_USAN "")
    #set(CXX_ASAN "")
endif()


enable_testing()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS} ${CXX_USAN} ${CXX_ASAN} ${CXX_IGNORE}")
set(CMAKE_CXX_STANDARD 20)

set(LSAN_OPTS "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/.lsan.supp")

#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*,bugprone*,clang-analyzer*,cppcoreguidelines*,performance*,portability*")

# --- Put all binaries in the root of the build dir ---
set(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${CMAKE_BINARY_DIR})

# --- External Packages ---

# --- OpenGL related packages --- #
find_package(OpenGL REQUIRED)
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/Common Files/MSVC/freeglut")
find_package(GLUT REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/Common Files/MSVC/glew-2.1.0")
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})

include(FetchContent)

# --- GLM --- #

FetchContent_Declare(
    glm 
    GIT_REPOSITORY https://github.com/g-truc/glm
)

FetchContent_MakeAvailable(glm)

# --- GLFW --- #
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
)

FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

endif()

if (NOT GLFW3_LIBRARY)
    set(GLFW3_LIBRARY glfw)
endif()

include_directories(${GLFW_SOURCE_DIR}/include)

# --- GOOGLETEST --- #
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- SDL 2  --- #
list(APPEND CMAKE_PREFIX_PATH "C:\\Program Files\\Common Files\\MSVC\\SDL2-2.0.22")

find_package(SDL2 REQUIRED)

# Required for SDL_mixer or #include "SDL.h" doesn't work
foreach (INCLUDE_DIR IN LISTS SDL2_INCLUDE_DIRS)
    include_directories(${INCLUDE_DIR})
    message("SDL DIR: " ${INCLUDE_DIR})
endforeach()
message ("LIB: " ${SDL2_LIBRARIES})
# Hack for SDL < 2.0.12
if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}" "${SDL2_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}" "${SDL2_LIBRARY}"
    )
endif()

if (NOT (TARGET SDL2::SDL2-static))
    message ("Didnt find SDL2::SDL2-static, adding dummy target")
    add_library(SDL2::SDL2-static ALIAS SDL2::SDL2)
endif()

if(NOT TARGET SDL2::SDL2main)
    message ("Didnt find SDL2::SDL2main, adding dummy target")
    add_library(SDL2::SDL2main ALIAS SDL2::SDL2)
endif()

# --- SDL2 Mixer X--- #

FetchContent_Declare(
    SDLMixerX
    GIT_REPOSITORY https://github.com/WohlSoft/SDL-Mixer-X
    GIT_TAG f7d4549e98afd6436791a70c1cf9b79781895dc1
)

# This package spams installs...
set(CMAKE_INSTALL_MESSAGE LAZY)

# For limpg123 find at http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win64.exe
list(APPEND CMAKE_PREFIX_PATH "C:\\Program Files\\Common Files\\MSVC\\yasm")


FetchContent_GetProperties(SDLMixerX)
if(NOT SDLMixerX_POPULATED)
    FetchContent_Populate(SDLMixerX)

    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS OFF)
    set(MIXERX_ENABLE_GPL ON)
    set(MIXERX_ENABLE_LGPL ON)
    set(SDL_MIXER_X_STATIC ON)
    set(SDL_MIXER_X_SHARED OFF)
    set(USE_MIDI_ADLMIDI ON)
    set(USE_MIDI_FLUIDSYNTH ON)
    set(USE_MIDI_FLUIDLITE OFF)
    set(USE_MIDI_TIMIDITY OFF)
    set(USE_MODPLUG OFF)
    set(USE_MP3_MPG123 OFF)
    set(USE_XMP OFF)
    set(USE_SYSTEM_SDL2 ON)
    set(DOWNLOAD_AUDIO_CODECS_DEPENDENCY ON)
    add_subdirectory(${sdlmixerx_SOURCE_DIR} ${sdlmixerx_BINARY_DIR})
endif()
include_directories(${SDLMixerX_SOURCE_DIR}/include)
link_directories(${SDLMixerX_BINARY_DIR}/lib)

# --- Common Link Libraries --- #
set(LINK_3D_LIBRARIES 
    ${GLEW_LIBRARIES} 
    glm
    ${OPENGL_LIBRARY} 
    ${GLFW3_LIBRARY}
    )

message ("System: ${CMAKE_SYSTEM}")
if (UNIX)
    set(LINK_UNIX_LIBRARIES
        stdc++
       m
        dl
        rt
        pthread)
endif()

# --- My project directories --- #
include_directories(".")

add_subdirectory("app")
add_subdirectory("audio")
add_subdirectory("bak")
add_subdirectory("com")
add_subdirectory("game")
add_subdirectory("graphics")
add_subdirectory("gui")
add_subdirectory("imgui")
add_subdirectory("shaders")
add_subdirectory("xbak")
