file(GLOB SOURCE_FILES *.cc)
file(GLOB HEADER_FILES *.h)
add_library(xbak ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(xbak
    com
    ${SDL_LIBRARIES}
    ${LINK_UNIX_LIBRARIES}
    #SDL_mixer 
    #SDL_sound
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
    add_executable (${XBAK_BIN} ${XBAK_BIN}.cc ${XBAK} ${SDL_INCLUDE_DIR} ${SDL_mixer_INCLUDE_DIR})
    target_link_libraries(${XBAK_BIN}
        xbak
        ${SDL_LIBRARIES}
        ${LINK_UNIX_LIBRARIES}
    )
endforeach()

add_executable (xbak_main xbak.cc ${XBAK} ${SDL_INCLUDE_DIR})
target_link_libraries(xbak_main
    xbak
    ${SDL_LIBRARIES}
    ${LINK_UNIX_LIBRARIES}
    )

set(CMAKE_CXX_CLANG_TIDY "")
