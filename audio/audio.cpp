#include "audio/audio.hpp"

#include "xbak/SoundResource.h"

namespace AudioA {

AudioManager& AudioManager::Get()
{
    static AudioManager audioManager{};
    return audioManager;
}

void AudioManager::ChangeMusicTrack(MusicIndex music)
{
    if (mCurrentMusicTrack && Mix_PlayingMusicStream(mCurrentMusicTrack))
    {
        Mix_FadeOutMusicStream(mCurrentMusicTrack, 1000);
    }

    mCurrentMusicTrack = GetMusic(music);
    Mix_PlayMusicStream(mCurrentMusicTrack, -1);
}

void AudioManager::PlaySound(SoundIndex sound)
{
    std::visit(overloaded{
        [](Mix_Music* music){
            Mix_PlayMusicStream(music, 1);
        },
        [](Mix_Chunk* chunk){
            
        }},
        GetSound(sound));
}

void AudioManager::StopMusicTrack()
{
    if (mCurrentMusicTrack)
    {
        Mix_HaltMusicStream(mCurrentMusicTrack);
        mCurrentMusicTrack = nullptr;
    }
}

Mix_Music* AudioManager::GetMusic(MusicIndex music)
{
    if (!mMusicData.contains(music))
    {
        auto& data = SoundResource::GetInstance()->GetSoundData(music.mValue);
        ASSERT(data.sounds.size() > 0);
        auto* fb = data.sounds[0]->GetSamples();
        auto* rwops = SDL_RWFromMem(fb->GetCurrent(), fb->GetSize());
        if (!rwops)
        {
            Logging::LogError("AudioManager") << SDL_GetError() << std::endl;
            //throw;
        }
        Mix_Music* musicData = Mix_LoadMUS_RW(rwops, 0);
        if (!musicData)
        {
            Logging::LogError("AudioManager") << Mix_GetError() << std::endl;
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
        auto& data = SoundResource::GetInstance()->GetSoundData(sound.mValue);
        ASSERT(data.sounds.size() > 0);
        auto* fb = data.sounds[0]->GetSamples();
        auto* rwops = SDL_RWFromMem(fb->GetCurrent(), fb->GetSize());
        if (!rwops)
        {
            Logging::LogError("AudioManager") << SDL_GetError() << std::endl;
            //throw;
        }
        Mix_Music* musicData = Mix_LoadMUS_RW(rwops, 0);
        if (!musicData)
        {
            Logging::LogError("AudioManager") << Mix_GetError() << std::endl;
        }

        Mix_SetMusicTempo(musicData, sMusicTempo);
        mSoundData[sound] = musicData;
    }

    return mSoundData[sound];
}

AudioManager::AudioManager()
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

AudioManager::~AudioManager()
{
    if (Mix_PlayingMusic())
    {
        Mix_FadeOutMusic(500);
        SDL_Delay(500);
        Mix_CloseAudio();
        SDL_Quit();
    }
}

}
