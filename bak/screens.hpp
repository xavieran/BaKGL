#include "bak/camera.hpp"
#include "bak/container.hpp"
#include "bak/constants.hpp"
#include "bak/dialog.hpp"
#include "bak/gameData.hpp"

#include "graphics/renderer.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

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
            ss << "Action :: " << action << std::endl;
        ImGui::TextWrapped(ss.str().c_str());
    }
    ImGui::TextWrapped("Text:\n %s", snippet.GetText().data());

    if (ImGui::Button("Back") && (!history.empty()))
    {
        current = history.top();
        history.pop();
    }

    for (const auto& choice : snippet.GetChoices())
    {
        std::stringstream ss{};
        ss << choice;
        if (ImGui::Button(ss.str().c_str()))
        {
            history.push(current);
            current = choice.mTarget;
        }
        //if (saveData != nullptr)
        //{
        //    ss.str(""); ss << "Val: " << saveData->ReadEvent(choice.mState);
        //    ImGui::SameLine(); ImGui::Text(ss.str().c_str());
        //}

        if (evaluate_if<BAK::KeyTarget>(choice.mTarget, [](const auto& current){ return current != BAK::KeyTarget{0}; }))
        {
            ImGui::TextWrapped(dialogStore.GetFirstText(
                dialogStore.GetSnippet(choice.mTarget)).substr(0, 40).data());
        }
        else
        {
            ImGui::TextWrapped("No text");
        }
    }

    ImGui::End();
}

void ShowCameraGui(
    const Camera& camera)
{
    const auto& pos = camera.GetPosition();
    ImGui::Begin("Info");
    std::stringstream ss{};
    ss << "Pos: " << pos
        << "\nHPos: " << std::hex << static_cast<std::uint32_t>(pos.x) 
        << ", " << static_cast<std::uint32_t>(-pos.z) << std::dec
        << "\nTile: " << glm::floor(camera.GetPosition() / glm::vec3{BAK::gTileSize})
        << "\nAngle: " << (360.0f * (camera.GetAngle() / (2.0f*3.141592f)));
    ImGui::TextWrapped(ss.str().c_str());
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f 
        / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void ShowContainerGui(
    const BAK::GenericContainer& container)
{
    std::cout << "Container: " << container << std::endl;
    std::stringstream ss{};
    ss << "\n" << container << "\n";
    ImGui::Text(ss.str().c_str());

    ImGui::BeginTable("Items", 4, ImGuiTableFlags_Resizable);
    ImGui::TableNextColumn(); ImGui::Text("Name");
    ImGui::TableNextColumn(); ImGui::Text("Cond/Qty");
    ImGui::TableNextColumn(); ImGui::Text("Status");
    ImGui::TableNextColumn(); ImGui::Text("Mods");
    ImGui::TableNextRow();

    for (const auto& item : container.GetInventory().GetItems())
    {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ss.str(""); ss << item.GetObject().mName; ImGui::Text(ss.str().c_str());
        ImGui::TableNextColumn();
        ss.str(""); ss << +item.GetCondition(); ImGui::Text(ss.str().c_str());
        ImGui::TableNextColumn();
        ss.str(""); ss << +item.GetStatus(); ImGui::Text(ss.str().c_str());
        ImGui::TableNextRow();
        ss.str(""); ss << +item.GetModifierMask(); ImGui::Text(ss.str().c_str());
        ImGui::TableNextRow();
    }
    ImGui::EndTable();
}


void ShowLightGui(
    Graphics::Light& light)
{
    ImGui::Begin("Light");
    ImGui::SliderFloat3("Dire", static_cast<float*>(&light.mDirection.x), -1.0f, 1.0f, "%.3f");
    ImGui::SliderFloat3("Ambi", static_cast<float*>(&light.mAmbientColor.x), .0f, 1.0f, "%.3f");
    ImGui::SliderFloat3("Diff", static_cast<float*>(&light.mDiffuseColor.x), .0f, 1.0f, "%.3f");
    ImGui::SliderFloat3("Spec", static_cast<float*>(&light.mSpecularColor.x), .0f, 1.0f, "%.3f");

    ImGui::End();
}
