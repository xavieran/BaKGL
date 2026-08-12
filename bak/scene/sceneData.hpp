#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#include <ostream>
#include <variant>

namespace BAK {

enum class AdsActions
{
    INDEX           = 0x1030,
    IF_NOT_PLAYED   = 0x1330,
    IF_PLAYED       = 0x1350,
    IF_CHAP_LTE     = 0x13a0,
    IF_CHAP_GTE     = 0x13b0,
    AND             = 0x1420,
    OR              = 0x1430,
    ELSE            = 0x1500,
    ADD_SCENE2      = 0x2000,
    ADD_SCENE       = 0x2005,
    STOP_SCENE      = 0x2010,
    PLAY_SCENE      = 0x1510,
    PLAY_ALL_SCENES = 0x1520,
    FADE_OUT        = 0xf010,
    END_IF          = 0xfff0,
    END             = 0xffff
};

std::string_view ToString(AdsActions a);
std::ostream& operator<<(std::ostream&, AdsActions);

enum class Actions
{
    SAVE_BACKGROUND     = 0x0020,
    UNKNOWN_A           = 0x0070,
    DRAW_BACKGROUND     = 0x0080,
    DRAW_BACKGROUND_B   = 0x00c0,
    PURGE               = 0x0110,
    // Purge means jump out of this scene
    // if no purge, then we continue looping through the
    // actions. This is how C12 archer works...
    UPDATE              = 0x0ff0,
    DO_SOMETHING_A      = 0x0400,
    // Don't clear sprites between updates...
    DISABLE_CLEAR       = 0x0500,
    ENABLE_CLEAR        = 0x0510,
    DELAY               = 0x1020,
    SLOT_IMAGE          = 0x1050,
    SLOT_PALETTE        = 0x1060,
    SLOT_FONT           = 0x1070,
    SET_SCENEA          = 0x1100, // Local taG?
    SET_SCENE           = 0x1110, // TAG??
    SET_SAVE_LAYER      = 0x1120,
    GOTO_TAG            = 0x1200,
    SET_COLOR           = 0x2000,
    SHOW_DIALOG         = 0x2010, 
    UNKNOWN3            = 0x2300,
    UNKNOWN6            = 0x2310,
    UNKNOWN7            = 0x2320,
    UNKNOWN4            = 0x2400,
    SET_CLIP_REGION     = 0x4000,
    FADE_OUT            = 0x4110,
    FADE_IN             = 0x4120,
    SAVE_IMAGE0         = 0x4200,
    SAVE_REGION_TO_LAYER = 0x4210,
    SET_WINDOWA         = 0xa010,
    SET_WINDOWB         = 0xa030,
    SET_UNKNOWN         = 0xa090,
    SET_WINDOWC         = 0xa0b0, // draw line?
    DRAW_RECT           = 0xa100,
    DRAW_FRAME          = 0xa110, 
    DRAW_TEXT           = 0xa200,
    DRAW_SPRITE0        = 0xa500,
    DRAW_SPRITE1        = 0xa510,
    DRAW_SPRITE_FLIP_Y  = 0xa520,
    DRAW_SPRITE_FLIP_XY = 0xa530,
    DRAW_SPRITE_ROTATE  = 0xa5a0,
    DRAW_SAVED_REGION    = 0xa600,
    DRAW_SCREEN         = 0xb600, // COPY_LAYER_TO_LAYER?
    LOAD_SOUND_RESOURCE = 0xc020,
    SELECT_SOUND        = 0xc030,
    DESELECT_SOUND      = 0xc040,
    PLAY_SOUND          = 0xc050,
    STOP_SOUND          = 0xc060,
    LOAD_SCREEN         = 0xf010,
    LOAD_IMAGE          = 0xf020,
    LOAD_FONT           = 0xf040,
    LOAD_PALETTE        = 0xf050
};

std::string_view ToString(Actions a);
std::ostream& operator<<(std::ostream&, Actions);

struct SetScene
{
    std::string mName;
    std::uint16_t mSceneNumber;
};

std::ostream& operator<<(std::ostream&, const SetScene&);

struct LoadScreen
{ 
    std::string mScreenName;
};

struct ClearScreen
{
};

struct DrawRect
{
    std::pair<unsigned, unsigned> mPaletteColor;
    glm::ivec2 mPos;
    glm::ivec2 mDims;
};

std::ostream& operator<<(std::ostream&, const DrawRect&);

struct DrawSprite
{
    bool mFlippedInY;
    std::int16_t mX;
    std::int16_t mY;
    std::int16_t mSpriteIndex;
    std::int16_t mImageSlot;
    std::int16_t mTargetWidth;
    std::int16_t mTargetHeight;
};

std::ostream& operator<<(std::ostream&, const DrawSprite&);

struct DrawScreen
{
    glm::ivec2 mPosition;
    glm::ivec2 mDimensions;
    unsigned mArg1;
    unsigned mArg2;
};

struct PlaySoundS
{
    unsigned mSoundIndex;
};

struct ClipRegion
{
    glm::vec<2, int> mTopLeft;
    glm::vec<2, int> mBottomRight;
};

std::ostream& operator<<(std::ostream& os, const ClipRegion& a);

struct DisableClipRegion
{
};

std::ostream& operator<<(std::ostream& os, const DisableClipRegion& a);

struct SaveBackground
{
};

struct DrawBackground 
{
};

struct GotoTag
{
    unsigned mTag;
};

struct SaveImage
{
    glm::ivec2 pos;
    glm::ivec2 dims;
};

struct SaveRegionToLayer
{
    glm::ivec2 pos;
    glm::ivec2 dims;
};

struct Purge
{
};

struct SetWindow
{
    std::uint16_t mX;
    std::uint16_t mY;
    std::uint16_t mWidth;
    std::uint16_t mHeight;
};


struct Delay
{
    unsigned mDelayMs; // units??
};

struct Update
{
};


struct SetColors
{
    unsigned mForegroundColor;
    unsigned mBackgroundColor;
};

struct FadeIn
{
};

struct FadeOut
{
};

struct SetSaveLayer
{
    unsigned mLayer;
};

struct DrawSavedRegion
{
    unsigned mLayer;
};

struct SlotImage
{
    unsigned mSlot;
};

struct LoadPalette
{
    std::string mPalette;
};

struct LoadImage
{
    std::string mImage;
};

struct SlotPalette
{
    unsigned mSlot;
};

struct ShowDialog
{
    unsigned mDialogKey;
    unsigned mDialogType;
};

using SceneAction = std::variant<
    ClearScreen,
    ClipRegion,
    Delay,
    DisableClipRegion,
    DrawBackground,
    DrawRect,
    DrawScreen,
    DrawSprite,
    Purge,
    SaveBackground,
    SaveImage,
    Update,
    LoadImage,
    LoadPalette,
    FadeOut,
    FadeIn,
    LoadScreen,
    SetScene,
    SetColors,
    SlotImage,
    SetSaveLayer,
    DrawSavedRegion,
    ShowDialog,
    PlaySoundS,
    SaveRegionToLayer,
    GotoTag,
    SlotPalette>;

std::ostream& operator<<(std::ostream& os, const SceneAction& sa);

}
