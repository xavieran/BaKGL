#include "bak/dialog.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/gameData.hpp"

#include "com/logger.hpp"

#include "imgui/imguiWrapper.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <stack>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::unique_ptr<BAK::GameData> gameData{nullptr};

    if (argc == 2)
        gameData = std::make_unique<BAK::GameData>(argv[1]);

    const auto& dialogStore = BAK::DialogStore::Get();

    if( !glfwInit() )
    {
        logger.Error() << "Failed to initialize GLFW" << std::endl;
        std::exit(1);
    }

    GLFWwindow* window;

    const unsigned antiAliasingSamples = 4;
    glfwWindowHint(GLFW_SAMPLES, antiAliasingSamples);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    unsigned height = 800;
    unsigned width  = 600;
    /*unsigned height = 1600;
    unsigned width  = 2400;*/

    window = glfwCreateWindow(width, height, argc == 2 ? argv[1] : "Dialog Explorer", NULL, NULL);
    if( window == NULL )
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        std::exit(1);
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        logger.Log(Logging::LogLevel::Error) << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    ImguiWrapper::Initialise(window);
    
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

        ImguiWrapper::Draw(window);
        
        // Swap buffers
        glfwSwapBuffers(window);
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS 
        && glfwWindowShouldClose(window) == 0);

    ImguiWrapper::Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

