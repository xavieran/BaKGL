#include "bak/sceneData.hpp"

#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <sstream>

namespace BAK {

std::string_view ToString(AdsActions a)
{
    switch (a)
    {
    case AdsActions::INDEX: return "Index";
    case AdsActions::IF_NOT_PLAYED: return "IfNotPlayed";
    case AdsActions::IF_PLAYED: return "IfPlayed";
    case AdsActions::IF_CHAP_LTE: return "IfChapterLTE";
    case AdsActions::ELSE: return "Else";
    case AdsActions::IF_CHAP_GTE: return "IfChapterGTE";
    case AdsActions::AND: return "And";
    case AdsActions::OR: return "Or";
    case AdsActions::ADD_SCENE2: return "AddScene2";
    case AdsActions::ADD_SCENE: return "AddScene";
    case AdsActions::STOP_SCENE: return "StopScene";
    case AdsActions::PLAY_SCENE: return "PlayScene";
    case AdsActions::PLAY_ALL_SCENES: return "PlayAllScenes";
    case AdsActions::FADE_OUT: return "FadeOut";
    case AdsActions::END_IF: return "EndIf";
    case AdsActions::END: return "End";
    default:
        std::stringstream ss{};
        ss << "Invalid ads action: 0x" << std::hex 
            << static_cast<int>(a) << "\n";
        throw std::runtime_error(ss.str());
    }
}

std::ostream& operator<<(std::ostream& os, AdsActions a)
{
    os << ToString(a);
    return os;
}
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
    case Actions::DISABLE_CLEAR: return "DisableClear";
    case Actions::ENABLE_CLEAR: return "EnableClear";
    case Actions::DELAY: return "Delay";
    case Actions::SLOT_IMAGE: return "SlotImage";
    case Actions::SLOT_PALETTE: return "SlotPalette";
    case Actions::SLOT_FONT: return "SlotFont";
    case Actions::SET_SCENE: return "SetScene";
    case Actions::SET_SCENEA: return "SETSCENEA";
    case Actions::SET_SAVE_LAYER: return "SetSaveLayer";
    case Actions::GOTO_TAG: return "GotoTag";
    case Actions::SET_COLOR: return "SetColors";
    case Actions::SHOW_DIALOG: return "ShowDialog";
    case Actions::FADE_OUT: return "FadeOut";
    case Actions::FADE_IN: return "FadeIn";
    case Actions::SAVE_IMAGE0: return "SaveImage0";
    case Actions::SAVE_REGION_TO_LAYER: return "SaveRegionToLayer";
    case Actions::SET_UNKNOWN: return "SETUNKNOWN";
    case Actions::SET_WINDOWA: return "SETWINDOWA";
    case Actions::SET_WINDOWB: return "SETWINDOWB";
    case Actions::SET_WINDOWC: return "SETWINDOWC";
    case Actions::DRAW_RECT: return "DrawRect";
    case Actions::DRAW_SPRITE0: return "DrawSprite";
    case Actions::DRAW_SPRITE1: return "DrawSprite1";
    case Actions::DRAW_SPRITE_FLIP_Y: return "DrawSpriteFlipY";
    case Actions::DRAW_SPRITE_FLIP_XY: return "DrawSpriteFlipXY";
    case Actions::DRAW_SPRITE_ROTATE: return "DrawSpriteRotate";
    case Actions::DRAW_SAVED_REGION: return "DrawSavedRegion";
    case Actions::DRAW_SCREEN: return "DrawScreen";
    case Actions::LOAD_SOUND_RESOURCE: return "LoadSoundResource";
    case Actions::SELECT_SOUND: return "SelectSound";
    case Actions::DESELECT_SOUND: return "DeselectSound";
    case Actions::PLAY_SOUND: return "PlaySound";
    case Actions::STOP_SOUND: return "StopSound";
    case Actions::LOAD_SCREEN: return "LoadScreen";
    case Actions::LOAD_IMAGE: return "LoadImage";
    case Actions::LOAD_PALETTE: return "LoadPalette";
    case Actions::LOAD_FONT: return "LoadFont";
    case Actions::SET_CLIP_REGION: return "SetClipRegion";
    case Actions::UNKNOWN_A: return "UNKNOWN_A";
    case Actions::UNKNOWN3: return "UNKNOWN3";
    case Actions::UNKNOWN4: return "UNKNOWN4";
    case Actions::DRAW_FRAME: return "DrawFrame";
    case Actions::DRAW_TEXT: return "DrawText";
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
    os << "SetScene {" << ss.mSceneNumber << " " << ss.mName << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoadScreen& ls)
{
    os << "LoadScreen {" << ls.mScreenName << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ClearScreen& ls)
{
    os << "ClearScreen {}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DrawRect& a)
{
    os << "DrawRect { PaletteColor: (" << std::get<0>(a.mPaletteColor)
        << ", " << std::get<1>(a.mPaletteColor)
        << "), Pos: " << a.mPos << ", Dims: "
        << a.mDims << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DrawScreen& a)
{
    os << "DrawScreen{ " << a.mPosition << " - " << a.mDimensions << " ("
        << a.mArg1 << ", " << a.mArg2 << ")}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DrawSprite& a)
{
    os << "DrawSprite{ flipped: " << a.mFlippedInY << " Pos { " << a.mX << ", " << a.mY
        << "} Sprite: " << a.mSpriteIndex << " imgSlot: " << a.mImageSlot
        << " Dims { " << a.mTargetWidth << ", " << a.mTargetHeight << " } }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ClipRegion& a)
{
    os << "ClipRegion{ TL: " << a.mTopLeft << " BR: " << a.mBottomRight << " } }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisableClipRegion& a)
{
    return os << "DisableClipRegion";
}

std::ostream& operator<<(std::ostream& os, const Delay& a)
{
    return os << "Delay { time: " << a.mDelayMs << "}";
}

std::ostream& operator<<(std::ostream& os, const Update& a)
{
    return os << "Update{}";
}

std::ostream& operator<<(std::ostream& os, const SaveBackground& a)
{
    return os << "SaveBackground{}";
}

std::ostream& operator<<(std::ostream& os, const DrawBackground& a)
{
    return os << "DrawBackground{}";
}

std::ostream& operator<<(std::ostream& os, const FadeIn& a)
{
    return os << "FadeIn{}";
}

std::ostream& operator<<(std::ostream& os, const FadeOut& a)
{
    return os << "FadeOut{}";
}

std::ostream& operator<<(std::ostream& os, const Purge& a)
{
    return os << "Purge{}";
}

std::ostream& operator<<(std::ostream& os, const SaveImage& a)
{
    return os << "SaveImage{ pos: " << a.pos << " dims: " << a.dims << "}";
}

std::ostream& operator<<(std::ostream& os, const SaveRegionToLayer& a)
{
    return os << "SaveRegionToLayer{ pos: " << a.pos << " dims: " << a.dims << "}";
}

std::ostream& operator<<(std::ostream& os, const LoadPalette& a)
{
    return os << "LoadPalette{ " << a.mPalette << "}";
}

std::ostream& operator<<(std::ostream& os, const LoadImage& a)
{
    return os << "LoadImage{ " << a.mImage << "}";
}

std::ostream& operator<<(std::ostream& os, const SlotImage& a)
{
    return os << "SlotImage{ " << a.mSlot << "}";
}

std::ostream& operator<<(std::ostream& os, const SetSaveLayer& a)
{
    return os << "SetSaveLayer{ " << a.mLayer << "}";
}

std::ostream& operator<<(std::ostream& os, const DrawSavedRegion& a)
{
    return os << "DrawSavedRegion{ " << a.mLayer << "}";
}

std::ostream& operator<<(std::ostream& os, const SetColors& a)
{
    return os << "SetColors{ fg: " << a.mForegroundColor << " bg: "
        << a.mBackgroundColor << "}";
}

std::ostream& operator<<(std::ostream& os, const SlotPalette& a)
{
    return os << "SlotPalette{ " << a.mSlot << "}";
}

std::ostream& operator<<(std::ostream& os, const ShowDialog& a)
{
    return os << "ShowDialog{ " << std::hex << a.mDialogKey << std::dec
        << " type: " << a.mDialogType << "}";
}

std::ostream& operator<<(std::ostream& os, const PlaySoundS& a)
{
    return os << "PlaySound { " << a.mSoundIndex << "}";
}

std::ostream& operator<<(std::ostream& os, const GotoTag& a)
{
    return os << "GotoTag{ " << a.mTag << "}";
}

std::ostream& operator<<(std::ostream& os, const SceneAction& sa)
{
    std::visit(overloaded{
        [&](const auto& x){ os << x; }},
        sa);

    return os;
}

}
