#include "bak/scene/sceneData.hpp"

#include "com/ostream.hpp"
#include "com/visit.hpp"

#include "graphics/glm.hpp"

#include <sstream>

namespace BAK {

std::string_view ToString(AdsActions a)
{
    switch (a)
    {
    case AdsActions::IF_NOT_PLAYED: return "IfNotPlayed";
    case AdsActions::IF_NOT_PLAYED_ELSE: return "IfNotPlayedElse";
    case AdsActions::IF_PLAYED_ELSE: return "IfPlayedElse";
    case AdsActions::IF_CHAP_LTE: return "IfChapterLTE";
    case AdsActions::ELSE: return "Else";
    case AdsActions::IF_CHAP_GTE: return "IfChapterGTE";
    case AdsActions::AND: return "And";
    case AdsActions::OR: return "Or";
    case AdsActions::RESTART_SCRIPT: return "RestartScript";
    case AdsActions::START_SCRIPT: return "StartScript";
    case AdsActions::STOP_SCRIPT: return "StopScript";
    case AdsActions::END_IF_ELSE: return "EndIfElse";
    case AdsActions::END_IF: return "EndIf";
    case AdsActions::STOP_SCENE: return "StopScene";
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
    case Actions::END_SCRIPT: return "EndScript";
    case Actions::END_FRAME: return "EndFrame";
    case Actions::DO_SOMETHING_A: return "DOSOMETHINGA";
    case Actions::DISABLE_CLEAR: return "DisableClear";
    case Actions::ENABLE_CLEAR: return "EnableClear";
    case Actions::DELAY: return "Delay";
    case Actions::SLOT_IMAGE: return "SlotImage";
    case Actions::SLOT_PALETTE: return "SlotPalette";
    case Actions::SLOT_FONT: return "SlotFont";
    case Actions::SET_SCRIPT: return "SetScript";
    case Actions::SCRIPT_TAG: return "ScriptTag";
    case Actions::SET_SAVE_LAYER: return "SetSaveLayer";
    case Actions::GOTO_TAG: return "GotoTag";
    case Actions::SET_COLOR: return "SetColors";
    case Actions::SHOW_DIALOG: return "ShowDialog";
    case Actions::FADE_OUT: return "FadeOut";
    case Actions::FADE_IN: return "FadeIn";
    case Actions::SAVE_RECT_TO_BACKGROUND: return "SaveRectToBackground";
    case Actions::SAVE_REGION_TO_LAYER: return "SaveRegionToLayer";
    case Actions::SET_UNKNOWN: return "SETUNKNOWN";
    case Actions::SET_WINDOWA: return "SETWINDOWA";
    case Actions::SET_WINDOWB: return "SETWINDOWB";
    case Actions::SET_WINDOWC: return "SETWINDOWC";
    case Actions::DRAW_RECT: return "DrawRect";
    case Actions::DRAW_SPRITE: return "DrawSprite";
    case Actions::DRAW_SPRITE_FLIP_Y: return "DrawSpriteFlipY";
    case Actions::DRAW_SPRITE_FLIP_X: return "DrawSpriteFlipX";
    case Actions::DRAW_SPRITE_FLIP_XY: return "DrawSpriteFlipXY";
    case Actions::DRAW_SPRITE_ROTATE: return "DrawSpriteRotate";
    case Actions::DRAW_SAVED_REGION: return "DrawSavedRegion";
    case Actions::COPY_LAYER: return "CopyLayer";
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

std::ostream& operator<<(std::ostream& os, const SetScript& ss)
{
    os << "SetScript {" << ss.mScriptNumber << " " << ss.mName << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ScriptTag& sl)
{
    os << "ScriptTag {" << sl.mScriptNumber << " " << sl.mName << "}";
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
    os << "DrawRect { PaletteColor: (" << a.mEdgeColor
        << ", " << a.mFillColor
        << "), Pos: " << a.mPos << ", Dims: "
        << a.mDims << ", filled: " << a.mFilled << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CopyLayer& a)
{
    os << "CopyLayer{ " << a.mPosition << " - " << a.mDimensions
        << " src: " << a.mSourceLayer << " dst: " << a.mTargetLayer << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DrawSprite& a)
{
    os << "DrawSprite{ flipX: " << a.mFlipX << " flipY: " << a.mFlipY
        << " Pos { " << a.mX << ", " << a.mY
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
    return os << "Delay { ticks: " << a.mTicks << "}";
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
    return os << "FadeIn{ start: " << a.mStartColor << " steps: " << a.mSteps
        << " end: " << a.mEndColor << " durationIndex: " << a.mDurationIndex << "}";
}

std::ostream& operator<<(std::ostream& os, const FadeOut& a)
{
    return os << "FadeOut{ start: " << a.mStartColor << " steps: " << a.mSteps
        << " end: " << a.mEndColor << " durationIndex: " << a.mDurationIndex << "}";
}

std::ostream& operator<<(std::ostream& os, const EndScript& a)
{
    return os << "EndScript{}";
}

std::ostream& operator<<(std::ostream& os, const SaveRectToBackground& a)
{
    return os << "SaveRectToBackground{ pos: " << a.pos << " dims: " << a.dims << "}";
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
    return os << "ShowDialog{ " << std::hex << a.mDialogKey
        << std::dec << " type: " << a.mDialogType << "}";
}

std::ostream& operator<<(std::ostream& os, const PlaySoundS& a)
{
    return os << "PlaySound { " << a.mSoundIndex << "}";
}

std::ostream& operator<<(std::ostream& os, const GotoTag& a)
{
    return os << "GotoTag{ " << a.mTag << "}";
}

std::ostream& operator<<(std::ostream& os, const ScriptAction& sa)
{
    std::visit(overloaded{
        [&](const auto& x){ os << x; }},
        sa);

    return os;
}

std::ostream& operator<<(std::ostream& os, const ScriptFrame& sf)
{
    os << "Frame";
    if (sf.mTag)
    {
        os << " [tag " << *sf.mTag << "]";
    }
    os << "\n";
    for (const auto& action : sf.mActions)
    {
        os << "\t" << action << "\n";
    }
    return os;
}

}
