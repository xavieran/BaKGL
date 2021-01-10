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

add_subdirectory("xbak")
include_directories("xbak")

include_directories("include")
include_directories("src")

set(BAK_INCLUDES
    xbak
)

set(LINK_3D_LIBRARIES 
    glfw
    ${GLEW_LIBRARIES} 
    ${GLM_LIBRARIES}
    ${OPENGL_LIBRARY}
)

set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-g -std=c++17 -Wall -Wextra -Werror ${CXX_IGNORE}")

list(APPEND BAK_BINARIES
    display_object
    main2d
    main3d
)

foreach (BAK_BIN IN LISTS BAK_BINARIES)
    add_executable (${BAK_BIN} src/${BAK_BIN}.cpp ${BAK_INCLUDES})
    target_link_libraries(${BAK_BIN} dl pthread ${Boost_LIBRARIES} ${LINK_3D_LIBRARIES} xbak SDL)
endforeach()

# XBaK Executables
list(APPEND XBAK_BINARIES
    dumpbmx
    "dumpzone_bmx"
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

