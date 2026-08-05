#include "audio/audio.hpp"

#include "bak/soundStore.hpp"

#include "com/visit.hpp"

#include "SDL_mixer.h"

#include <mutex>
#include <utility>

namespace AudioA {

MidiPlayer StringToMidiPlayer(std::string_view player)
{
    if (player == "ADLMIDI")
    {
        return MidiPlayer::ADLMIDI;
    }
    else if (player == "OPNMIDI")
    {
        return MidiPlayer::OPNMIDI;
    }
    else if (player == "FluidSynth")
    {
        return MidiPlayer::FluidSynth;
    }
    else
    {
        Logging::LogError(__FUNCTION__) << "Not a valid midi player (" << player << ") valid options are: ADLMIDI, OPNMIDI, FluidSynth, defaulting to ADLMIDI\n";
        return MidiPlayer::ADLMIDI;
    }
}

AudioManagerProvider::AudioManagerProvider()
:
    mAudioManager{std::make_unique<NullAudioManager>()}
{
}

AudioManagerProvider& AudioManagerProvider::Get()
{
    static AudioManagerProvider provider{};
    return provider;
}

IAudioManager& AudioManagerProvider::GetAudioManager()
{
    assert(mAudioManager);
    return *mAudioManager;
}

void AudioManagerProvider::SetAudioManager(std::unique_ptr<IAudioManager>&& manager)
{
    Get().mAudioManager = std::move(manager);
}

IAudioManager& GetAudioManager()
{
    return AudioManagerProvider::Get().GetAudioManager();
}

AudioManager::AudioManager()
:
    mLogger{Logging::LogState::GetLogger("AudioManager")}
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        mLogger.Error() << "Couldn't initialize SDL: "
            << SDL_GetError() << std::endl;
    }

    if (Mix_OpenAudio(sAudioRate, sAudioFormat, sAudioChannels, sAudioBuffers) < 0)
    {
        mLogger.Error() << "Couldn't initialize SDL: "
            << SDL_GetError() << std::endl;
    }

    Mix_VolumeMusic(sAudioVolume);
    Mix_SetMidiPlayer(MIDI_OPNMIDI);

    mAudioThread = std::thread{[this]{ AudioLoop(); }};
}

AudioManager& AudioManager::Get()
{
    if (!sStaticAudioManager)
    {
        Logging::LogFatal(__FUNCTION__) << "Audio manager singleton not set" << std::endl;
        throw std::runtime_error("Audio manager singleton not set");
    }
    return *sStaticAudioManager;
}

void AudioManager::Set(AudioManager* audioManager)
{
    sStaticAudioManager = audioManager;
}

void AudioManager::EnqueueCommand(Command cmd)
{
    {
        std::lock_guard<std::mutex> lock(mCommandMutex);
        mCommandQueue.emplace_back(std::move(cmd));
    }
    mCommandCv.notify_one();
}

void AudioManager::AudioLoop()
{
    while (true)
    {
        Command cmd;
        {
            std::unique_lock<std::mutex> lock(mCommandMutex);
            mCommandCv.wait(lock, [this]{ return !mCommandQueue.empty(); });
            cmd = std::move(mCommandQueue.front());
            mCommandQueue.pop_front();
        }

        if (std::holds_alternative<CommandShutdown>(cmd))
        {
            DoShutdown();
            return;
        }

        std::visit(overloaded{
            [this](CommandPlaySound c){ DoPlaySound(c.sound); },
            [this](CommandChangeTrack c){ DoChangeTrack(c.music); },
            [this](CommandPopTrack){ DoPopTrack(); },
            [this](CommandStopTrack){ DoStopTrack(); },
            [this](CommandSwitchPlayer c){ DoSwitchPlayer(c.player); },
            [this](CommandSoundFinished c){ DoSoundFinished(c.music); },
            [](CommandShutdown){}},
            cmd);
    }
}

void AudioManager::DoShutdown()
{
    ClearSounds();
    Mix_CloseAudio();
}

void AudioManager::ChangeMusicTrack(MusicIndex musicI)
{
    mLogger.Debug() << "Changing track to: " << musicI << "\n";
    EnqueueCommand(CommandChangeTrack{musicI});
}

void AudioManager::PlayTrack(Mix_Music* music)
{
    if (music == mCurrentMusicTrack)
    {
        return;
    }

    const auto startTime = Mix_GetMusicLoopStartTime(music);

    if (mCurrentMusicTrack && Mix_PlayingMusicStream(mCurrentMusicTrack))
    {
        Mix_CrossFadeMusicStream(mCurrentMusicTrack, music, -1, sFadeOutTime, 0);
    }
    else
    {
        Mix_FadeInMusicStream(music, -1, sFadeOutTime);
    }

    mCurrentMusicTrack = music;
}

void AudioManager::DoChangeTrack(MusicIndex musicI)
{
    if (!mMusicStack.empty())
    {
        mMusicStack.pop();
    }

    auto* music = GetMusic(musicI);
    mMusicStack.push(music);
    mLogger.Debug() << "Changing track to: " << musicI
        << " stack size: " << mMusicStack.size() << "\n";

    PlayTrack(music);
}

void AudioManager::PopTrack()
{
    EnqueueCommand(CommandPopTrack{});
}

void AudioManager::DoPopTrack()
{
    if (mMusicStack.empty())
    {
        mLogger.Debug() << "Popping music track, stack already empty\n";
        return;
    }

    if (mMusicStack.size() == 1)
    {
        mLogger.Debug() << "Popping music track, stack now empty\n";
        auto* music = mMusicStack.top();
        mMusicStack.pop();
        mCurrentMusicTrack = nullptr;
        Mix_FadeOutMusicStream(music, sFadeOutTime);
    }
    else
    {
        mLogger.Debug() << "Popping music track, stack size: " << mMusicStack.size() << "\n";
        mMusicStack.pop();
        PlayTrack(mMusicStack.top());
    }
}

void AudioManager::PlaySound(SoundIndex sound)
{
    mLogger.Debug() << "Queueing sound: " << sound << "\n";
    EnqueueCommand(CommandPlaySound{sound});
}

void AudioManager::DoPlaySound(SoundIndex sound)
{
    if (mSoundPlaying)
    {
        mPendingSounds.emplace(sound);
    }
    else
    {
        PlaySoundImpl(sound);
    }
}

void AudioManager::PlaySoundImpl(SoundIndex sound)
{
    mLogger.Debug()  << "Playing sound: " << sound << "\n";

    std::visit(overloaded{
        [this](Mix_Music* music){
            mCurrentSound = music;
            mSoundPlaying = true;
            Mix_HookMusicStreamFinished(music, &AudioManager::SoundFinishedHook, this);
            Mix_PlayMusicStream(music, 1);
        },
        [](Mix_Chunk* chunk){
            
        }},
        GetSound(sound));
}

void AudioManager::SoundFinishedHook(Mix_Music* music, void* self)
{
    auto* audioManager = static_cast<AudioManager*>(self);
    audioManager->EnqueueCommand(CommandSoundFinished{music});
}

void AudioManager::DoSoundFinished(Mix_Music* music)
{
    if (music != mCurrentSound)
        return;

    mSoundPlaying = false;
    mCurrentSound = nullptr;

    auto it = std::find_if(
        mSoundData.begin(),
        mSoundData.end(),
        [music](const auto& sound)
        {
            return std::holds_alternative<Mix_Music*>(sound.second) 
                && std::get<Mix_Music*>(sound.second) == music;
        });

    if (it != mSoundData.end())
    {
        Mix_FreeMusic(music);
        mSoundData.erase(it);
    }

    if (!mPendingSounds.empty())
    {
        auto sound = mPendingSounds.front();
        mPendingSounds.pop();
        PlaySoundImpl(sound);
    }
}

void AudioManager::DoStopTrack()
{
    if (mCurrentMusicTrack)
    {
        Mix_FadeOutMusicStream(mCurrentMusicTrack, 2000);
        mCurrentMusicTrack = nullptr;
    }
}

void AudioManager::DoSwitchPlayer(MidiPlayer midiPlayer)
{
    ClearSounds();

    switch (midiPlayer)
    {
    case MidiPlayer::ADLMIDI:
        Mix_SetMidiPlayer(MIDI_ADLMIDI);
        break;
    case MidiPlayer::OPNMIDI:
        Mix_SetMidiPlayer(MIDI_OPNMIDI);
        break;
    case MidiPlayer::FluidSynth:
        Mix_SetMidiPlayer(MIDI_Fluidsynth);
        break;
    default:
        std::unreachable();
    }
}

void AudioManager::ClearSounds()
{
    mCurrentMusicTrack = nullptr;
    mPendingSounds = {};

    for (auto& [_, music] : mMusicData)
    {
        Mix_HaltMusicStream(music);
        Mix_FreeMusic(music);
    }
    mMusicData.clear();

    for (auto& [_, sound] : mSoundData)
    {
        std::visit(overloaded{
            [](Mix_Music* music){
                Mix_HaltMusicStream(music);
                Mix_FreeMusic(music);
            },
            [](Mix_Chunk* chunk){
                Mix_FreeChunk(chunk);
            }},
            sound);
    }
    mSoundData.clear();

    while (!mMusicStack.empty()) mMusicStack.pop();

    mSoundPlaying = false;
    mCurrentSound = nullptr;

    {
        std::lock_guard<std::mutex> lock(mCommandMutex);
        std::erase_if(mCommandQueue, [](const Command& c){
            return std::holds_alternative<CommandSoundFinished>(c);
        });
    }
}

void AudioManager::StopMusicTrack()
{
    EnqueueCommand(CommandStopTrack{});
}

Mix_Music* AudioManager::GetMusic(MusicIndex music)
{
    if (!mMusicData.contains(music))
    {
        auto& data = BAK::SoundStore::Get().GetSoundData(music.mValue);
        ASSERT(data.GetSounds().size() > 0);
        auto* fb = data.GetSounds()[0].GetSamples();
        auto* rwops = SDL_RWFromMem(fb->GetCurrent(), fb->GetSize());
        if (!rwops)
        {
            mLogger.Error() << SDL_GetError() << std::endl;
        }
        Mix_Music* musicData = Mix_LoadMUS_RW(rwops, 0);
        if (!musicData)
        {
            mLogger.Error() << Mix_GetError() << std::endl;
        }

        Mix_SetMusicTempo(musicData, sMusicTempo);

        mMusicData[music] = musicData;
    }

    return mMusicData[music];
}

AudioManager::Sound AudioManager::GetSound(SoundIndex sound)
{
    if (!mSoundData.contains(sound))
    {
        auto& data = BAK::SoundStore::Get().GetSoundData(sound.mValue);
        ASSERT(data.GetSounds().size() > 0);
        auto* fb = data.GetSounds()[0].GetSamples();
        auto* rwops = SDL_RWFromMem(fb->GetCurrent(), fb->GetSize());
        if (!rwops)
        {
            mLogger.Error() << SDL_GetError() << std::endl;
        }
        Mix_Music* musicData = Mix_LoadMUS_RW(rwops, 0);
        if (!musicData)
        {
            mLogger.Error() << Mix_GetError() << std::endl;
        }

        Mix_SetMusicTempo(musicData, sMusicTempo);
        mSoundData[sound] = musicData;
    }

    return mSoundData[sound];
}

void AudioManager::SwitchMidiPlayer(MidiPlayer midiPlayer)
{
    EnqueueCommand(CommandSwitchPlayer{midiPlayer});
}

AudioManager::~AudioManager()
{
    EnqueueCommand(CommandShutdown{});
    mAudioThread.join();
    SDL_Quit();
}

AudioManager* AudioManager::sStaticAudioManager{nullptr};

}
