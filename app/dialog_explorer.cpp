#include "bak/dialog.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/gameData.hpp"

#include "com/logger.hpp"

#include "graphics/glfw.hpp"

#include "imgui/imguiWrapper.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stack>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::unique_ptr<BAK::GameData> gameData{nullptr};

    if (argc == 2)
        gameData = std::make_unique<BAK::GameData>(argv[1]);

    const auto& dialogStore = BAK::DialogStore::Get();

    auto window = Graphics::MakeGlfwWindow(
        800,
        600,
        argc == 2 ? argv[1] : "Dialog Explorer");

    ImguiWrapper::Initialise(window.get());
    
    // Dark blue background
    glClearColor(0.15f, 0.31f, 0.36f, 0.0f);


    auto history = std::stack<BAK::Target>{};
    //auto current = dialogIndex.GetKeys().front();
    auto current = BAK::Target{BAK::KeyTarget{0x72}};
    
    do
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            if (!dialogStore.HasSnippet(current)) current = BAK::KeyTarget{0x72};
            const auto& snippet = dialogStore.GetSnippet(current);

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            bool use_work_area = true;
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            ImGui::Begin("Dialog");
            //ImGui::Text("Dialog indices: %zu", dialogIndex.GetKeys().size());
            static char curIndex[64] = "0"; ImGui::InputText("Index ", curIndex, 10); //, ImGuiInputTextFlags_CharsHex);
            ImGui::SameLine(); if (ImGui::Button("Change"))
            {
                while (!history.empty()) history.pop();
                //current = dialogIndex.GetKeys()[atoi(curIndex)];Q
                std::stringstream ss{};
                ss << std::hex << curIndex;
                std::uint32_t key;
                ss >> key;
                current = BAK::KeyTarget{key};
                logger.Info() << "Change Key to: " << current << std::endl;
            }

            {
                std::stringstream ss{};
                ss << "Previous: ";
                if (ImGui::Button("Back") && (!history.empty()))
                {
                    current = history.top();
                    history.pop();
                }
                if (!history.empty()) ss << history.top();
                else ss << "[None]";
                ss << " Current: " << current;

                ImGui::Text(ss.str().c_str());
            }
            {
                std::stringstream ss{};
                ss << "[ ds: " << std::hex << +snippet.mDisplayStyle << " act: " << +snippet.mActor
                    << " ds2: " << +snippet.mDisplayStyle2 << " ds3: " << +snippet.mDisplayStyle3 << " ]" << std::endl;
                for (const auto& action : snippet.mActions)
                    ss << "Action :: " << action << std::endl;
                ImGui::TextWrapped(ss.str().c_str());
            }

            ImGui::TextWrapped("Text:\n %s", snippet.GetText().data());

            for (const auto& choice : snippet.GetChoices())
            {
                std::stringstream ss{};
                ss << choice;
                if (ImGui::Button(ss.str().c_str()))
                {
                    history.push(current);
                    current = choice.mTarget;
                }
                //if (gameData != nullptr)
                //{
                //    ss.str(""); ss << "Val: " << gameData->ReadEvent(choice.mState) << " Word: " << std::hex << gameData->ReadEventWord(choice.mState)
                //        << " @state: " << choice.mState;
                //    ImGui::SameLine(); ImGui::Text(ss.str().c_str());
                //}
                if (choice.mTarget != BAK::Target{BAK::KeyTarget{0}})
                {
                    ImGui::TextWrapped(dialogStore.GetFirstText(
                        dialogStore.GetSnippet(choice.mTarget)).substr(0, 40).data());
                }
            }

            ImGui::End();
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        ImguiWrapper::Draw(window.get());
        
        // Swap buffers
        glfwSwapBuffers(window.get());
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window.get()) == 0);

    ImguiWrapper::Shutdown();

    return 0;
}

