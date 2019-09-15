cmake_minimum_required(VERSION 2.8)
project( BaKSandbox )

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.45.0 COMPONENTS optional serialization archive)

include_directories("include")
include_directories(src)
include_directories(${Boost_INCLUDE_DIRS}) 

set(LOGURU
    include/loguru.hpp)

set(BAK_INCLUDES
    ${LOGURU}
)

set(CMAKE_CXX_FLAGS "-g -fstack-protector-all  -Og -std=c++14 -Wall -Wno-unused-variable -Wno-switch")

#-static

add_executable (main src/main.cpp src/loguru.cpp ${BAK_INCLUDES})
target_link_libraries(main dl pthread ${Boost_LIBRARIES} boost_serialization)
