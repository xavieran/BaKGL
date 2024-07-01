#include "bak/backgroundSounds.hpp"

#include "audio/audio.hpp"
#include "bak/gameState.hpp"

#include "bak/types.hpp"
#include "com/random.hpp"

namespace BAK {

void PlayUndergroundSound() {
    if (GetRandomNumber(0, 0xfff) % 384 == 0)
    {
        AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{3});
        //unsigned duration = (GetRandomNumber(0, 0xfff) % 50) + 10;
    }
}

void PlayNightTimeSound()
{
    unsigned soundIndex;
    if (GetRandomNumber(0, 0xfff) % 100 <= 5)
    {
        soundIndex = 0x5A;
    }
    else
    {
        soundIndex = 0x33;
    }
    AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{soundIndex});
    //unsigned duration = (GetRandomNumber(0, 0xfff) % 54) + 10;
}

void PlayZoneTwoSound()
{
    unsigned soundIndex;
    if (GetRandomNumber(0, 0xfff) % 100 <= 50)
    {
        soundIndex = 0x85; // gulls
    }
    else
    {
        soundIndex = (GetRandomNumber(0, 0xfff) % 2) + 0x35;
    }
    AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{soundIndex});
    //unsigned duration = (GetRandomNumber(0, 0xfff) % 54) + 10;
}

void PlayRegularSound()
{
    unsigned soundIndex = (GetRandomNumber(0, 0xfff) % 3) + 0x34;
    AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{soundIndex});
}

void PlayBackgroundSounds(GameState& gs)
{
    // Is underground...
    if (gs.GetZone().mValue > 9)
    {
        PlayUndergroundSound();
    }
    else if (gs.GetChapter() != Chapter{8} && gs.GetZone() != ZoneNumber{6})
    {
        if (GetRandomNumber(0, 0xfff) % 110 == 0)
        {
            if (!gs.ReadEventBool(0x753a))
            {
                PlayNightTimeSound();
            }
            else
            {
                if (gs.GetZone() == ZoneNumber{2})
                {
                    PlayZoneTwoSound();
                }
                else
                {
                    PlayRegularSound();
                }
            }
        }
    }
}

}
