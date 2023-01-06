
add_library(audio
    audio.hpp audio.cpp
)

if (MSVC)
	find_library(SDL_LIB SDL2)
else()
	set(SDL_LIB SDL2)
endif()

target_link_libraries(audio
    bak
    ${SDL_LIB}
    SDL2_mixer_ext_Static
)
