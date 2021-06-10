#include "bak/sceneData.hpp"

#include <sstream>

namespace BAK {

std::string_view ToString(Actions a)
{
    switch (a)
    {
    case Actions::SAVE_BACKGROUND: return "SaveBackground";
    case Actions::DRAW_BACKGROUND: return "DrawBackground";
    case Actions::DRAW_BACKGROUND_B: return "DRAW_BACKGROUND_B";
    case Actions::PURGE: return "Purge";
    case Actions::UPDATE: return "Update";
    case Actions::DO_SOMETHING_A: return "DOSOMETHINGA";
    case Actions::DO_SOMETHING_B: return "DOSOMETHINGB";
    case Actions::DO_SOMETHING_C: return "DOSOMETHINGC";
    case Actions::DELAY: return "Delay";
    case Actions::SLOT_IMAGE: return "SlotImage";
    case Actions::SLOT_PALETTE: return "SlotPalette";
    case Actions::SLOT_UNKNOWN: return "SLOTUNKNOWN";
    case Actions::SET_SCENE: return "SetScene";
    case Actions::SET_SCENEA: return "SETSCENEA";
    case Actions::SET_SCENEB: return "SETSCENEB";
    case Actions::SET_SCENEC: return "SETSCENEC";
    case Actions::SET_COLORS: return "SetColors";
    case Actions::SET_FRAME1: return "SetFrame1";
    case Actions::FADE_OUT: return "FadeOut";
    case Actions::FADE_IN: return "FadeIn";
    case Actions::SAVE_IMAGE0: return "SaveImage0";
    case Actions::SAVE_IMAGE1: return "SaveImage1";
    case Actions::SET_UNKNOWN: return "SETUNKNOWN";
    case Actions::SET_WINDOWA: return "SETWINDOWA";
    case Actions::SET_WINDOWB: return "SETWINDOWB";
    case Actions::SET_WINDOWC: return "SETWINDOWC";
    case Actions::DRAW_RECT: return "DrawRect";
    case Actions::DRAW_SPRITE0: return "DrawSprite0";
    case Actions::DRAW_SPRITE1: return "DrawSprite1";
    case Actions::DRAW_SPRITE_FLIP: return "DrawSpriteFlip";
    case Actions::DRAW_SPRITE3: return "DrawSprite3";
    case Actions::DRAW_SPRITEA: return "DRAWSPRITEA";
    case Actions::DRAW_SPRITEB: return "DRAWSPRITEB";
    case Actions::DRAW_SCREEN: return "DrawScreen";
    case Actions::LOAD_SOUND_RESOURCE: return "LoadSoundResource";
    case Actions::SELECT_SOUND: return "SelectSound";
    case Actions::DESELECT_SOUND: return "DeselectSound";
    case Actions::PLAY_SOUND: return "PlaySound";
    case Actions::STOP_SOUND: return "StopSound";
    case Actions::LOAD_SCREEN: return "LoadScreen";
    case Actions::LOAD_IMAGE: return "LoadImage";
    case Actions::LOAD_PALETTE: return "LoadPalette";
    case Actions::LOAD_UNKNOWN: return "LOAD_UNKNOWN";
    case Actions::SET_CLIP_REGION: return "SetClipRegion";
    case Actions::UNKNOWN_A: return "UNKNOWN_A";
    case Actions::UNKNOWN3: return "UNKNOWN3";
    case Actions::UNKNOWN4: return "UNKNOWN4";
    case Actions::UNKNOWN5: return "UNKNOWN5";
    case Actions::UNKNOWN6: return "UNKNOWN6";
    case Actions::UNKNOWN7: return "UNKNOWN7";
    default:
        std::stringstream ss{};
        ss << "Invalid action: 0x" << std::hex 
            << static_cast<int>(a) << "\n";
        throw std::runtime_error(ss.str());
    }
}

std::ostream& operator<<(std::ostream& os, Actions a)
{
    os << ToString(a);
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetScene& ss)
{
    os << "SetScene{" << ss.mSceneNumber << " " << ss.mName << "}";
    return os;
}

}


