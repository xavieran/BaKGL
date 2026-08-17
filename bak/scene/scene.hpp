#pragma once

#include "bak/scene/sceneData.hpp"
#include "bak/types.hpp"

#include <optional>
#include <vector>
#include <unordered_map>

namespace BAK {

class FileBuffer;

struct ImageSlot
{
    std::vector<std::string> mImage;
    std::optional<unsigned> mPalette;
};

using PaletteSlot = unsigned;

struct Script
{
    std::string mSceneTag;
    std::vector<ScriptAction> mActions;
    std::unordered_map<PaletteSlot, std::string> mPalettes;
    std::unordered_map<unsigned, std::pair<std::string, PaletteSlot>> mImages;
    std::unordered_map<PaletteSlot, std::pair<std::string, PaletteSlot>> mScreens;

    std::optional<ClipRegion> mClipRegion;
};

std::ostream& operator<<(std::ostream&, const Script&);

std::unordered_map<unsigned, Script> LoadScripts(FileBuffer& fb);
std::vector<ScriptFrame> LoadDynamicScripts(FileBuffer& fb);

FileBuffer DecompressSCR(FileBuffer& fb);
FileBuffer DecompressTT3(FileBuffer& fb);
FileBuffer DecompressTTM(FileBuffer& fb);

void DumpADS(FileBuffer& fb, std::ostream& os);
void DumpTTM(FileBuffer& fb, std::ostream& os);

}
