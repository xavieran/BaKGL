#include "audio/audio.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    
    std::string soundOrMusic{argv[1]};
    std::string soundIndexStr{argv[2]};

    logger.Info() << "Sound index:" << soundIndexStr << std::endl;

    std::stringstream ss{};
    ss << soundIndexStr;
    unsigned index;
    ss >> index;

    if (soundOrMusic == "m")
    {
        AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{index});
    }
    if (soundOrMusic == "s")
    {
        AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{index});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds{5000});
    return 0;
}

