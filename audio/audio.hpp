#pragma once

#include "com/logger.hpp"
#include "com/strongType.hpp"
#include "com/visit.hpp"

#include <SDL2/SDL.h>
#include "SDL_mixer_ext/SDL_mixer_ext.h"

#include <chrono>
#include <ostream>
#include <queue>
#include <stack>
#include <thread>
#include <variant>
#include <memory>
#include <unordered_map>

namespace AudioA {

static constexpr auto MIN_SOUND = 1;
static constexpr auto MAX_SOUND = 134;
static constexpr auto MIN_SONG = 1001;
static constexpr auto MAX_SONG = 1063;
using SoundIndex = Bounded<StrongType<unsigned, struct SoundIndexTag>, MIN_SOUND, MAX_SOUND>;
using MusicIndex = Bounded<StrongType<unsigned, struct MusicIndexTag>, MIN_SONG, MAX_SONG>;

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

MidiPlayer StringToMidiPlayer(std::string_view);

class IAudioManager
{
public:
    virtual void ChangeMusicTrack(MusicIndex) = 0;
    virtual void PopTrack() = 0;
    virtual void PauseMusicTrack() = 0;
    virtual void PlayMusicTrack() = 0;
    virtual void StopMusicTrack() = 0;
    virtual void PlaySound(SoundIndex) = 0;
    virtual void SwitchMidiPlayer(MidiPlayer) = 0;

    virtual ~IAudioManager() {}
};

class NullAudioManager : public IAudioManager
{
public:
    void ChangeMusicTrack(MusicIndex) override {}
    void PopTrack() override {}
    void PauseMusicTrack() override {}
    void PlayMusicTrack() override {}
    void StopMusicTrack() override {}
    void PlaySound(SoundIndex) override {}
    void SwitchMidiPlayer(MidiPlayer) override {}
};

class AudioManagerProvider
{
public:
    static AudioManagerProvider& Get();
    IAudioManager& GetAudioManager();
    static void SetAudioManager(std::unique_ptr<IAudioManager>&&);
private:
    AudioManagerProvider();

    std::unique_ptr<IAudioManager> mAudioManager;
};

IAudioManager& GetAudioManager();

class AudioManager : public IAudioManager
{
    static constexpr auto sAudioRate{MIX_DEFAULT_FREQUENCY};
    static constexpr auto sAudioFormat{MIX_DEFAULT_FORMAT};
    static constexpr auto sAudioChannels{MIX_DEFAULT_CHANNELS};
    static constexpr auto sAudioBuffers{4096};
    static constexpr auto sAudioVolume{MIX_MAX_VOLUME};
    static constexpr auto sMusicTempo{0.9};
    static constexpr auto sFadeOutTime{1500};

    using Sound = std::variant<Mix_Music*, Mix_Chunk*>;

public:
    AudioManager();

    static AudioManager& Get();
    static void Set(AudioManager*);

    void ChangeMusicTrack(MusicIndex) override;
    void PopTrack() override;
    void PauseMusicTrack() override {}
    void PlayMusicTrack() override {}
    void StopMusicTrack() override;

    void PlaySound(SoundIndex) override;

    void SwitchMidiPlayer(MidiPlayer) override;

    ~AudioManager();
private:
    void PlayTrack(Mix_Music* music);
    void PlaySoundImpl(SoundIndex);

    Sound GetSound(SoundIndex);

    Mix_Music* GetMusic(MusicIndex);

    static void RewindMusic(Mix_Music*, void*);

    void ClearSounds();

    Mix_Music* mCurrentMusicTrack;
    std::stack<Mix_Music*> mMusicStack;
    std::queue<SoundIndex> mSoundQueue;
    bool mSoundPlaying;

    std::unordered_map<SoundIndex, Sound> mSoundData;
    std::unordered_map<MusicIndex, Mix_Music*> mMusicData;

    bool mRunning;
    std::thread mQueuePlayThread;
    static AudioManager* sStaticAudioManager;

    const Logging::Logger& mLogger;
    
};

}
