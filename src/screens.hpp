#include "dialog.hpp"
#include "logger.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <stack>

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
    unsigned index,
    const auto& dialogStore,
    const auto& dialogIndex)
{
    static unsigned currentIndex = 0;
    static auto history = std::stack<BAK::Target>{};
    static auto current = dialogIndex.GetKeys()[currentIndex];

    if (index != currentIndex)
    {
        currentIndex = index;
        current = dialogIndex.GetKeys()[index];
    }

    const auto& snippet = dialogStore.GetSnippet(current);

    ImGui::Begin("Dialog");
    ImGui::Text("Dialog indices: %d", dialogIndex.GetKeys().size());
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
