#pragma once

#include "bak/dialogTarget.hpp"

class Camera;
namespace BAK {
class DialogStore;
class GameData;
class GenericContainer;
}

namespace Graphics {
struct Light;
}

void ShowDialogGui(
    BAK::Target dialogKey,
    const BAK::DialogStore& dialogStore,
    const BAK::GameData* saveData = nullptr);

void ShowCameraGui(
    const Camera& camera);

void ShowContainerGui(
    const BAK::GenericContainer& container);

void ShowLightGui(
    Graphics::Light& light);
