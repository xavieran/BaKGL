#include "camera.hpp"
#include "constants.hpp"
#include "dialog.hpp"
#include "logger.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <stack>
#include <sstream>

//FIX!
template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& a)
{
    std::string sep = "";
    for (unsigned i = 0; i < N; i++)
    {
        os << sep << std::setw(2) << std::setfill('0') << +a[i];
        sep = " ";
    }
    return os;
}

void ShowDialogGui(
    BAK::Target dialogKey,
    const auto& dialogStore,
    const auto& dialogIndex)
{
    static auto chosenKey = BAK::Target{BAK::KeyTarget{0x72}};
    static auto current = BAK::Target{BAK::KeyTarget{0x72}};
    static auto history = std::stack<BAK::Target>{};

    if (chosenKey != dialogKey)
    {
        chosenKey = dialogKey;
        current = dialogKey;
    }

    const auto& snippet = dialogStore.GetSnippet(current);

    ImGui::Begin("Dialog");

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

void ShowDialogGuiIndex(
    unsigned index,
    const auto& dialogStore,
    const auto& dialogIndex)
{
    static unsigned currentIndex = 0;
    static BAK::Target current = dialogIndex.GetKeys()[currentIndex];

    if (index != currentIndex)
    {
        currentIndex = index;
        current = dialogIndex.GetKeys()[index];
    }

    ShowDialogGui(current, dialogStore, dialogIndex);
}


void ShowCameraGui(
    const Camera& camera)
{
    ImGui::Begin("Info");
    std::stringstream ss{};
    ss << "Pos: " << camera.GetPosition() 
        << "\nNPos: " << camera.GetNormalisedPosition()
        << "\nTile: " << glm::floor(camera.GetPosition() / glm::vec3{BAK::gTileSize});
    ImGui::Text(ss.str().c_str());
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f 
        / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

}
