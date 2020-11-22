cmake_minimum_required(VERSION 3.13)
project( BaKSandbox )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/boost)

set(BOOST_REQUESTED_VERSION 1.72.0)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.72.0 COMPONENTS serialization)
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_subdirectory("xbak")
include_directories("xbak")


include_directories("include")
include_directories("src")

set(LOGURU
    include/loguru.hpp)

set(BAK_INCLUDES
    ${LOGURU}
)

set(CXX_IGNORE "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-g -fstack-protector-all -std=c++17 -Wall -Wextra -Werror ${CXX_IGNORE}")

#-static

add_executable (main src/main.cpp src/loguru.cpp ${BAK_INCLUDES} ${XBAK})
target_link_libraries(main dl pthread ${Boost_LIBRARIES} boost_serialization xbak)

add_executable (xbak_main xbak/xbak.cc ${XBAK} ${SDL_INCLUDE_DIR})
target_link_libraries(xbak_main dl pthread ${Boost_LIBRARIES} boost_serialization xbak SDL)
