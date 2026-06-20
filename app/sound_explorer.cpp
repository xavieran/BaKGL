#include "audio/audio.hpp"

#include "bak/soundStore.hpp"

#include "com/logger.hpp"

#include "imgui/imguiWrapper.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "graphics/glfw.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    auto& provider = AudioA::AudioManagerProvider::Get();
    auto audioManager = std::make_unique<AudioA::AudioManager>();
    audioManager->Set(audioManager.get());
    audioManager->SwitchMidiPlayer(AudioA::StringToMidiPlayer("ADLMIDI"));
    provider.SetAudioManager(std::move(audioManager));

    BAK::SoundStore::Get();

    auto window = Graphics::MakeGlfwWindow(800, 600, "Sound Explorer");

    ImguiWrapper::Initialise(window.get());

    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);

    const char* midiPlayers[] = { "ADLMIDI", "OPNMIDI", "FluidSynth" };
    int currentMidiPlayer = 0;

    do
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            bool use_work_area = true;
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            ImGui::Begin("Sound Explorer");

            if (ImGui::BeginTabBar("Tabs"))
            {
                if (ImGui::BeginTabItem("Sounds"))
                {
                    ImGui::BeginChild("SoundsList");
                    for (unsigned i = AudioA::MIN_SOUND; i < AudioA::MAX_SOUND; i++)
                    {
                        std::string label{};
                        try
                        {
                            auto& data = BAK::SoundStore::Get().GetSoundData(i);
                            label = std::to_string(i) + ": " + data.GetName();
                        }
                        catch (const std::out_of_range&)
                        {
                            label = std::to_string(i) + ": <UNKNOWN>";
                        }

                        if (ImGui::Button(label.c_str()))
                        {
                            logger.Info() << "Playing sound: " << i << std::endl;
                            AudioA::AudioManager::Get().StopMusicTrack();
                            AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{i});
                        }
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Music"))
                {
                    ImGui::BeginChild("MusicList");
                    for (unsigned i = AudioA::MIN_SONG; i < AudioA::MAX_SONG; i++)
                    {
                        std::string label{};
                        try
                        {
                            auto& data = BAK::SoundStore::Get().GetSoundData(i);
                            label = std::to_string(i) + ": " + data.GetName();
                        }
                        catch (const std::out_of_range&)
                        {
                            label = std::to_string(i) + ": <UNKNOWN>";
                        }

                        if (ImGui::Button(label.c_str()))
                        {
                            logger.Info() << "Playing music: " << i << std::endl;
                            AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{i});
                        }
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::Text("MIDI Player");
                    if (ImGui::Combo("##midi", &currentMidiPlayer, midiPlayers, IM_ARRAYSIZE(midiPlayers)))
                    {
                        AudioA::AudioManager::Get().SwitchMidiPlayer(
                            static_cast<AudioA::MidiPlayer>(currentMidiPlayer));
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        glClear(GL_COLOR_BUFFER_BIT);

        ImguiWrapper::Draw(window.get());

        glfwSwapBuffers(window.get());
    }
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    return 0;
}
