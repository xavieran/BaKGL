#include "audio/audio.hpp"

#include "com/logger.hpp"

#undef main

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    
    std::string soundOrMusic{argv[1]};
    std::string soundIndexStr{argv[2]};

    logger.Info() << "Sound index:" << soundIndexStr << std::endl;

    auto& provider = AudioA::AudioManagerProvider::Get();
    auto audioManager = std::make_unique<AudioA::AudioManager>();
    audioManager->Set(audioManager.get());
    audioManager->SwitchMidiPlayer(AudioA::StringToMidiPlayer("ADLMIDI"));
    provider.SetAudioManager(std::move(audioManager));

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

