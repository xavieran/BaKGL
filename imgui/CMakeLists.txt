file(GLOB SOURCE_FILES *.cpp)
file(GLOB HEADER_FILES *.h)
add_library(imgui ${SOURCE_FILES} ${HEADER_FILES})
set(CMAKE_CXX_CLANG_TIDY "")
