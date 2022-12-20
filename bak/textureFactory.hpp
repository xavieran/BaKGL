#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "bak/image.hpp"
#include "bak/palette.hpp"

#include "graphics/texture.hpp"

#include "bak/fileBuffer.hpp"

namespace BAK {

class TextureFactory
{
public:
    static Graphics::TextureStore MakeTextureStore(
        std::string_view bmx,
        std::string_view pal);

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
        const BAK::Image& terrain,
        const BAK::Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore& store,
        const BAK::Image& image,
        const Palette& palette);

    static void AddToTextureStore(
        Graphics::TextureStore& store,
        const std::vector<BAK::Image>& images,
        const Palette& palette);
};

}
