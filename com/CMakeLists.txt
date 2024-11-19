add_library(com
    algorithm.hpp
    bmpWriter.hpp
    demangle.hpp demangle.cpp
    getopt.h getopt_long.c
    json.hpp json_fwd.hpp
    logger.hpp logger.cpp
    path.hpp path.cpp
    png.hpp png.cpp
    random.hpp random.cpp
    string.hpp string.cpp
    stb_image.hpp
    visit.hpp
    ostream.hpp
    ostreamMux.hpp ostreamMux.cpp
)

if (ENABLE_CPPTRACE)
    target_link_libraries(com
        cpptrace::cpptrace
    )
endif()
