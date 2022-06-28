#pragma once

#include "com/logger.hpp"
#include "com/strongType.hpp"
#include "com/visit.hpp"

#include <SDL2/SDL.h>
#include "SDL_mixer_ext/SDL_mixer_ext.h"

#include <ostream>
#include <queue>
#include <stack>
#include <variant>

namespace AudioA {

using SoundIndex = Bounded<StrongType<unsigned, struct SoundIndexTag>, 1, 134>;
using MusicIndex = Bounded<StrongType<unsigned, struct MusicIndexTag>, 1001, 1063>;

static constexpr auto PUZZLE_CHEST_THEME = MusicIndex{1003};
static constexpr auto BAD_BARD  = MusicIndex{1008};
static constexpr auto POOR_BARD = MusicIndex{1040};
static constexpr auto GOOD_BARD = MusicIndex{1039};
static constexpr auto BEST_BARD = MusicIndex{1007};

enum class MidiPlayer
{
    ADLMIDI = 0,
    OPNMIDI = 1,
    FluidSynth = 2
};

class AudioManager
{
    static constexpr auto sAudioRate{MIX_DEFAULT_FREQUENCY};
    static constexpr auto sAudioFormat{MIX_DEFAULT_FORMAT};
    static constexpr auto sAudioChannels{MIX_DEFAULT_CHANNELS};
    static constexpr auto sAudioBuffers{4096};
    static constexpr auto sAudioVolume{MIX_MAX_VOLUME};
    static constexpr auto sMusicTempo{0.9};

    using Sound = std::variant<Mix_Music*, Mix_Chunk*>;

public:
    static AudioManager& Get();

    void ChangeMusicTrack(MusicIndex);
    void PauseMusicTrack();
    void PlayMusicTrack();
    void StopMusicTrack();

    void PlaySound(SoundIndex);
    void PlaySoundImpl(SoundIndex);

    void SwitchMidiPlayer(MidiPlayer);

private:
    Sound GetSound(SoundIndex);

    Mix_Music* GetMusic(MusicIndex);

    static void RewindMusic(Mix_Music*, void*);

    void ClearSounds();

    AudioManager();
    ~AudioManager();

    Mix_Music* mCurrentMusicTrack;
    std::stack<Mix_Music*> mMusicStack;
    std::queue<SoundIndex> mSoundQueue;
    bool mSoundPlaying;

    std::unordered_map<SoundIndex, Sound> mSoundData;
    std::unordered_map<MusicIndex, Mix_Music*> mMusicData;
    
};

}
