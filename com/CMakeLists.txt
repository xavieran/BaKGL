add_library(com
    algorithm.hpp
    demangle.hpp demangle.cpp
    getopt.h getopt_long.c
    json.hpp json_fwd.hpp
    logger.hpp logger.cpp
    path.hpp path.cpp
    png.hpp png.cpp pngWrite.cpp
    random.hpp random.cpp
    string.hpp string.cpp
    stb_image.h
    stb_image_write.h
    visit.hpp
    ostream.hpp
    ostreamMux.hpp ostreamMux.cpp
)

if (ENABLE_CPPTRACE)
    target_link_libraries(com
        cpptrace::cpptrace
    )
endif()
