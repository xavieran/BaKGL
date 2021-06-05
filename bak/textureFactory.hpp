#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "graphics/texture.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"
#include "xbak/ImageResource.h"
#include "xbak/PaletteResource.h"
#include "xbak/ScreenResource.h"

namespace BAK {

Graphics::Texture ImageToTexture(
    const Image& image,
    const Palette& palette);

class TextureFactory
{
public:
    static Graphics::TextureStore MakeTextureStore(
        std::string_view bmx,
        std::string_view pal);

    static Graphics::TextureStore MakeTextureStore(
        const ImageResource&,
        const Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        const ImageResource&,
        const Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore&,
        const ScreenResource&,
        const Palette&);

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
        const Palette&);

};

}
