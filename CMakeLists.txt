cmake_minimum_required(VERSION 2.8)
project( fLaCPGA )

include_directories("include")
include_directories(src)

set(LOGURU
    include/loguru.hpp)

set(BAK_INCLUDES
    ${LOGURU}
)

set(CMAKE_CXX_FLAGS "-g -fstack-protector-all  -Og -std=c++14 -Wall -Wno-unused-variable -Wno-switch")

#-static

add_executable (main src/main.cpp src/loguru.cpp ${BAK_INCLUDES})
target_link_libraries(main dl pthread)
