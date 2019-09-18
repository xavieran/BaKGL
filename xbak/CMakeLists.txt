file(GLOB SOURCE_FILES *.cc)
file(GLOB HEADER_FILES *.h)
add_library(xbak ${SOURCE_FILES} ${HEADER_FILES})
