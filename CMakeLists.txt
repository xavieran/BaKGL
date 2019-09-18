cmake_minimum_required(VERSION 3.13)
project( BaKSandbox )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/boost)

set(BOOST_REQUESTED_VERSION 1.71.0)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71.0 COMPONENTS serialization)

add_subdirectory("xbak")

include_directories(${Boost_INCLUDE_DIRS}) 

include_directories("include")
include_directories("src")

set(LOGURU
    include/loguru.hpp)

set(BAK_INCLUDES
    ${LOGURU}
)

set(CMAKE_CXX_FLAGS "-g -fstack-protector-all  -Og -std=c++14 -Wall -Wno-unused-variable -Wno-switch")

#-static

add_executable (main src/main.cpp src/loguru.cpp ${BAK_INCLUDES} ${XBAK})
target_link_libraries(main dl pthread ${Boost_LIBRARIES} boost_serialization xbak)
