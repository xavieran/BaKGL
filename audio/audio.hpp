#pragma once

#include "com/logger.hpp"
#include "com/strongType.hpp"
#include "com/visit.hpp"

#include <SDL2/SDL.h>
#include "SDL_mixer_ext/SDL_mixer_ext.h"

#include <ostream>
#include <variant>

namespace AudioA {

using SoundIndex = Bounded<StrongType<unsigned, struct SoundIndexTag>, 1, 134>;
using MusicIndex = Bounded<StrongType<unsigned, struct MusicIndexTag>, 1001, 1063>;

static constexpr auto PUZZLE_CHEST_THEME = MusicIndex{1003};
static constexpr auto BAD_BARD  = MusicIndex{1008};
static constexpr auto POOR_BARD = MusicIndex{1040};
static constexpr auto GOOD_BARD = MusicIndex{1039};
static constexpr auto BEST_BARD = MusicIndex{1007};

class AudioManager
{
    static constexpr auto sAudioRate{MIX_DEFAULT_FREQUENCY};
    static constexpr auto sAudioFormat{MIX_DEFAULT_FORMAT};
    static constexpr auto sAudioChannels{MIX_DEFAULT_CHANNELS};
    static constexpr auto sAudioBuffers{4096};
    static constexpr auto sAudioVolume{MIX_MAX_VOLUME};
    static constexpr auto sMusicTempo{0.9};

public:
    static AudioManager& Get();

    void ChangeMusicTrack(MusicIndex);
    void PauseMusicTrack();
    void PlayMusicTrack();
    void StopMusicTrack();

    void PlaySound(SoundIndex);

private:
    using Sound = std::variant<Mix_Music*, Mix_Chunk*>;
    Sound GetSound(SoundIndex);

    Mix_Music* GetMusic(MusicIndex);

    AudioManager();
    ~AudioManager();

    Mix_Music* mCurrentMusicTrack;
    std::unordered_map<SoundIndex, Sound> mSoundData;
    std::unordered_map<MusicIndex, Mix_Music*> mMusicData;
    
};

}
