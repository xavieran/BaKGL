file(GLOB SOURCE_FILES *.cc)
file(GLOB HEADER_FILES *.h)
add_library(xbak ${SOURCE_FILES} ${HEADER_FILES})
    
set_target_properties(xbak PROPERTIES CMAKE_CXX_CLANG_TIDY "")
target_link_libraries(xbak
    com
    glm
    SDL2
    SDL2_mixer_ext_Static
    ${LINK_UNIX_LIBRARIES}
)

# XBaK Executables
list(APPEND XBAK_BINARIES
    bmx2bmp
    dumpads
    dumpbmx
    dumpact_bmx
    dumpfnt
    dumpchar_bmx
    dumpzone_bmx
    dumppal
    dumpreq
    dumpobj
    dumptbl
    dumpscx
    dumpwld
    resourcedemo
)

foreach (XBAK_BIN IN LISTS XBAK_BINARIES)
    add_executable (${XBAK_BIN} ${XBAK_BIN}.cc ${XBAK})
    target_link_libraries(${XBAK_BIN}
        xbak
        ${LINK_UNIX_LIBRARIES}
    )
endforeach()

add_executable (xbak_main xbak.cc ${XBAK})
target_link_libraries(xbak_main
    xbak
    ${LINK_UNIX_LIBRARIES}
    )

set(CMAKE_CXX_CLANG_TIDY "")
