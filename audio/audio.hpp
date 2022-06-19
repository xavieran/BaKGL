#pragma once

#include "com/logger.hpp"

#include "SDL_Audio.h"

#include <SDL2/SDL.h>
#include "SDL_mixer.h"

#include <ostream>

namespace Audio {

class AudioManager
{
    static constexpr auto sAudioRate{MIX_DEFAULT_FREQUENCY};
    static constexpr auto sAudioFormat{MIX_DEFAULT_FORMAT};
    static constexpr auto sAudioChannels{MIX_DEFAULT_CHANNELS};
    static constexpr auto sAudioBuffers{4096};
    static constexpr auto sAudioVolume{MIX_MAX_VOLUME};

    AudioManager()
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            Logging::LogDebug("Audio") << "Couldn't initialize SDL: "
                << SDL_GetError() << std::endl;
        }

        if (Mix_OpenAudio(sAudioRate, sAudioFormat, sAudioChannels, sAudioBuffers) < 0)
        {
            Logging::LogDebug("Audio") << "Couldn't initialize SDL: "
                << SDL_GetError() << std::endl;
        }

        Mix_VolumeMusic(sAudioVolume);
    }

    ~AudioManager()
    {
        if (Mix_PlayingMusic())
        {
            Mix_FadeOutMusic(500);
            SDL_Delay(500);
            Mix_CloseAudio();
            SDL_Quit();
        }
    }
};

}
