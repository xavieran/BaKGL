#pragma once

#include <string_view>
#include <vector>

namespace Graphics {
class TextureStore;
}

namespace BAK {

class Image;
class Palette;

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
        const Image& terrain,
        const Palette&);

    static void AddToTextureStore(
        Graphics::TextureStore& store,
        const Image& image,
        const Palette& palette);

    static void AddToTextureStore(
        Graphics::TextureStore& store,
        const std::vector<Image>& images,
        const Palette& palette);
};

}
