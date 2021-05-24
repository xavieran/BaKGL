file(GLOB SOURCE_FILES *.cpp)
file(GLOB HEADER_FILES *.hpp)

add_library(
    graphics
    ${SOURCE_FILES} ${HEADER_FILES})
