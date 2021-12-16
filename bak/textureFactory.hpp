#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "bak/palette.hpp"

#include "graphics/texture.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/ImageResource.h"
#include "xbak/ScreenResource.h"

namespace BAK {

Graphics::Texture ImageToTexture(
    const Image& image,
    const BAK::Palette& palette);

class TextureFactory
{
public:
    static Graphics::TextureStore MakeTextureStore(
        std::string_view bmx,
        std::string_view pal);

    static Graphics::TextureStore MakeTextureStore(
        const ImageResource&,
        const BAK::Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        const ImageResource&,
        const BAK::Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        const ImageResource&,
        const BAK::Palette&,
        unsigned imageIndex);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        const ScreenResource&,
        const BAK::Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        std::string_view bmx,
        std::string_view pal);

    static void AddScreenToTextureStore(
        Graphics::TextureStore&,
        std::string_view scx,
        std::string_view pal);

    static void AddTerrainToTextureStore(
        Graphics::TextureStore&,
        const ScreenResource&,
        const BAK::Palette&);

};

}
