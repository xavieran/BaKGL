
add_library(audio
    audio.hpp audio.cpp
)

target_link_libraries(audio
    xbak
    SDL2
    SDL2_mixer_ext_Static
)
