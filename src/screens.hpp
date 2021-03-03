#include "camera.hpp"
#include "container.hpp"
#include "constants.hpp"
#include "dialog.hpp"
#include "gameData.hpp"
#include "logger.hpp"
#include "ostream.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <stack>
#include <sstream>

void ShowDialogGui(
    BAK::Target dialogKey,
    const auto& dialogStore,
    const auto& dialogIndex,
    const BAK::GameData* saveData = nullptr)
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
        if (saveData != nullptr)
        {
            ss.str(""); ss << "Val: " << saveData->ReadEvent(choice.mState);
            ImGui::SameLine(); ImGui::Text(ss.str().c_str());
        }

        ImGui::TextWrapped(dialogStore.GetFirstText(
            dialogStore.GetSnippet(choice.mTarget)).substr(0, 40).c_str());
    }

    ImGui::End();
}

void ShowDialogGuiIndex(
    unsigned index,
    const auto& dialogStore,
    const auto& dialogIndex,
    const BAK::GameData* saveData = nullptr)
{
    static unsigned currentIndex = 0;
    static BAK::Target current = dialogIndex.GetKeys()[currentIndex];

    if (index != currentIndex)
    {
        currentIndex = index;
        current = dialogIndex.GetKeys()[index];
    }

    ShowDialogGui(current, dialogStore, dialogIndex, saveData);
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

void ShowContainerGui(
    const BAK::Container& container)
{
    std::stringstream ss{};
    ss << "\nAddr: " << std::hex << container.mAddress << " #" << std::dec 
        << container.mNumber << " items: " << container.mNumberItems << " cap: " 
        << container.mCapacity << " type: " << container.mType;
    ImGui::Text(ss.str().c_str());

    ImGui::BeginTable("Items", 4, ImGuiTableFlags_Resizable);
    ImGui::TableNextColumn(); ImGui::Text("Number");
    ImGui::TableNextColumn(); ImGui::Text("Name");
    ImGui::TableNextColumn(); ImGui::Text("Cond/Qty");
    ImGui::TableNextColumn(); ImGui::Text("Mods");
    ImGui::TableNextRow();

    for (const auto& item : container.mItems)
    {
        ImGui::TableNextColumn();
        ss.str(""); ss << item.mItemNumber; ImGui::Text(ss.str().c_str());
        ImGui::TableNextColumn();
        ss.str(""); ss << item.mName; ImGui::Text(ss.str().c_str());
        ImGui::TableNextColumn();
        ss.str(""); ss << +item.mCondition; ImGui::Text(ss.str().c_str());
        ImGui::TableNextColumn();
        ss.str(""); ss << +item.mModifiers; ImGui::Text(ss.str().c_str());
        ImGui::TableNextRow();
    }
    ImGui::EndTable();
}
