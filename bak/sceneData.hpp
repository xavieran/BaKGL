#pragma once

#include "com/visit.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#include <ostream>
#include <variant>


namespace BAK {

enum class AdsActions
{
    INDEX         = 0x1030,
    IF_NOT_PLAYED = 0x1330,
    IF_PLAYED     = 0x1350,
    UNKNOWN       = 0x13a0,
    UNKNOWN3      = 0x13b0,
    AND           = 0x1420,
    OR            = 0x1430,
    UNKNOWN2      = 0x1500,
    ADD_SCENE2    = 0x2000,
    ADD_SCENE     = 0x2005,
    STOP_SCENE    = 0x2010,
    PLAY_SCENE    = 0x1510,
    PLAY_SCENE2   = 0x1520,
    FADE_OUT      = 0xf010,
    END_IF        = 0xfff0,
    END           = 0xffff
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
    UPDATE              = 0x0ff0,
    DO_SOMETHING_A      = 0x0400,
    DO_SOMETHING_B      = 0x0500,
    DO_SOMETHING_C      = 0x0510,
    DELAY               = 0x1020,
    SLOT_IMAGE          = 0x1050,
    SLOT_PALETTE        = 0x1060,
    SLOT_UNKNOWN        = 0x1070,
    SET_SCENEA          = 0x1100, // Local taG?
    SET_SCENE           = 0x1110, // TAG??
    SET_SCENEB          = 0x1120, // SET BACKGROUND
    SET_SCENEC          = 0x1200, // GOTO TAG
    SET_COLORS          = 0x2000, // SET_COLORS
    SET_FRAME1          = 0x2010, 
    UNKNOWN3            = 0x2300,
    UNKNOWN6            = 0x2310,
    UNKNOWN7            = 0x2320,
    UNKNOWN4            = 0x2400,
    SET_CLIP_REGION     = 0x4000, // SET_CLIP_REGION
    FADE_OUT            = 0x4110,
    FADE_IN             = 0x4120,
    SAVE_IMAGE0         = 0x4200, // DRAW_BACKGROUND_REGION
    SAVE_IMAGE1         = 0x4210, // SAVE_IMIAGE_REGION
    SET_UNKNOWN         = 0xa090,
    SET_WINDOWA         = 0xa010,
    SET_WINDOWB         = 0xa030,
    SET_WINDOWC         = 0xa0b0, // draw line?
    DRAW_RECT           = 0xa100,
    DRAW_FRAME          = 0xa110, 
    DRAW_SPRITE0        = 0xa500,
    DRAW_SPRITE1        = 0xa510,
    DRAW_SPRITE_FLIP    = 0xa520,
    DRAW_SPRITE3        = 0xa530,
    DRAW_SPRITEA        = 0xa5a0,
    DRAW_SPRITEB        = 0xa600, // CLEAR_SCREEN
    DRAW_SCREEN         = 0xb600,
    LOAD_SOUND_RESOURCE = 0xc020,
    SELECT_SOUND        = 0xc030,
    DESELECT_SOUND      = 0xc040,
    PLAY_SOUND          = 0xc050,
    STOP_SOUND          = 0xc060,
    LOAD_SCREEN         = 0xf010,
    LOAD_IMAGE          = 0xf020,
    LOAD_UNKNOWN        = 0xf040,
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

struct DrawRect
{
    std::int16_t mTLX;
    std::int16_t mTLY;
    std::int16_t mBRX;
    std::int16_t mBRY;
};

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

struct PlaySound
{
    std::uint16_t mSoundIndex;
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

struct SaveImage
{
    std::uint16_t mX;
    std::uint16_t mY;
    std::uint16_t mWidth;
    std::uint16_t mHeight;
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
    std::uint16_t mDelayMs; // units??
};

struct SetColors
{
    unsigned mSlot; // which slot palette to pull colors from
    std::uint16_t mForegroundColor;
    std::uint16_t mBackgroundColor;
};

struct FadeOut
{
    
};

struct Purge
{
    
};

using SceneAction = std::variant<
    DrawSprite,
    ClipRegion,
    DisableClipRegion>;

std::ostream& operator<<(std::ostream& os, const SceneAction& sa);

}
