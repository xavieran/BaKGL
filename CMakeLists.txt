cmake_minimum_required(VERSION 3.19)
project( BaKSandbox )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/boost)

set(BOOST_REQUESTED_VERSION 1.72.0)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.72.0 COMPONENTS adaptors)
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_subdirectory(glad)

add_subdirectory("xbak")
include_directories("xbak")

include_directories("include")
include_directories("src")

set(LOGURU
    #include/loguru.hpp
)

set(BAK_INCLUDES
    xbak
    ${LOGURU}
)

set(LINK_3D_LIBRARIES 
    glfw
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-g -fstack-protector-all -std=c++17 -Wall -Wextra -Werror ${CXX_IGNORE}")


add_executable (main3d src/main3d.cpp src/loguru.cpp ${BAK_INCLUDES})
target_link_libraries(main3d dl pthread ${Boost_LIBRARIES} ${LINK_3D_LIBRARIES} ${BAK_INCLUDES} SDL)

add_executable (main2d src/main.cpp src/loguru.cpp ${BAK_INCLUDES} ${XBAK})
target_link_libraries(main2d dl pthread ${Boost_LIBRARIES} xbak SDL)

# XBaK Executables
list(APPEND XBAK_BINARIES
    dumpbmx
    dumppal
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

