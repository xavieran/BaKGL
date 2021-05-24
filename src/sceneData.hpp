#pragma once

#include <cstdint>
#include <string>

#include <variant>


namespace BAK {

enum class Actions
{
    SAVE_BACKGROUND    = 0x0020,
    DRAW_BACKGROUND    = 0x0080,
    PURGE              = 0x0110,
    UPDATE             = 0x0ff0,
    DELAY              = 0x1020,
    SLOT_IMAGE         = 0x1050,
    SLOT_PALETTE       = 0x1060,
    // SLOT_SOUND = 0x1090, MAYBE
    SET_SCENE          = 0x1110,
    SET_FRAME0         = 0x2000,
    SET_FRAME1         = 0x2010,
    FADE_OUT           = 0x4110,
    FADE_IN            = 0x4120,
    SAVE_IMAGE0        = 0x4200,
    SAVE_IMAGE1        = 0x4210,
    SET_WINDOW         = 0xa100,
    DRAW_SPRITE0       = 0xa500,
    DRAW_SPRITE1       = 0xa510,
    DRAW_SPRITE2       = 0xa520,
    DRAW_SPRITE3       = 0xa530,
    DRAW_SCREEN        = 0xb600,
    LOAD_SOUNDRESOURCE = 0xc020,
    SELECT_SOUND       = 0xc030,
    DESELECT_SOUND     = 0xc040,
    PLAY_SOUND         = 0xc050,
    STOP_SOUND         = 0xc060,
    LOAD_SCREEN        = 0xf010,
    LOAD_IMAGE         = 0xf020,
    LOAD_PALETTE       = 0xf050
    // LOAD_SOUND = 0xf090, MAYBE
};

struct SetScene
{
    std::string mName;
    std::int16_t mSceneNumber;
};

struct SlotPalette
{
    std::int16_t mSlot;
};

struct LoadPalette
{ 
    std::string mPaletteName;
};

struct LoadScreen
{ 
    std::string mScreenName;
};

struct SlotImage
{ 
    std::int16_t mSlot;
};

struct LoadImage
{
    std::string mImageName;
};

struct DrawSprite0
{
    std::int16_t mX;
    std::int16_t mY;
    std::int16_t mSpriteIndex;
    std::int16_t mImageSlot;
};

// DrawSprite1 ??

struct DrawSprite2
{
    std::int16_t mX;
    std::int16_t mY;
    std::int16_t mSpriteIndex;
    std::int16_t mImageSlot;
    std::int16_t mNotSure1;
    std::int16_t mNotSure2;
};

struct PlaySound
{
    std::uint16_t mSoundIndex;
};

struct Update
{

};

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

struct SetFrame0
{
    // Not sure on these values
    std::uint16_t mX;
    std::uint16_t mY;
};

struct FadeOut
{
    
};

struct Purge
{
    
};

using SceneAction = std::variant<
    SetScene,
    SlotPalette,
    LoadPalette,
    LoadScreen,
    SlotImage,
    LoadImage,
    DrawSprite0,
    DrawSprite2,
    PlaySound,
    Update,
    SaveImage,
    SetWindow,
    Delay,
    SetFrame0,
    FadeOut,
    Purge>;

}
