#include "dialog.hpp"
#include "logger.hpp"
#include "ostream.hpp"

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
    
    BAK::ZoneLabel zoneLabel{"Z01"};

    BAK::DialogStore dialogStore{};
    dialogStore.Load();
    BAK::DialogIndex dialogIndex{zoneLabel};
    dialogIndex.Load();

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

    window = glfwCreateWindow(width, height, "BaK", NULL, NULL);
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
    auto current = dialogIndex.GetKeys().front();
    
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
            ImGui::Text("Dialog indices: %zu", dialogIndex.GetKeys().size());
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
                    ss << "Action :: " << action.mAction << std::endl;
                ImGui::TextWrapped(ss.str().c_str());
            }
            ImGui::TextWrapped("Text:\n %s", snippet.GetText().c_str());

            if (ImGui::Button("Back") && (!history.empty()))
            {
                current = history.top();
                history.pop();
            }

            for (const auto& choice : snippet.GetChoices())
            {
                std::stringstream ss{};
                ss << std::hex << choice.mState << " " << choice.mChoice1 
                    << " " << choice.mChoice2 << " " << choice.mTarget;
                if (ImGui::Button(ss.str().c_str()))
                {
                    history.push(current);
                    current = choice.mTarget;
                }

                ImGui::TextWrapped(dialogStore.GetFirstText(
                    dialogStore.GetSnippet(choice.mTarget)).substr(0, 40).c_str());
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

