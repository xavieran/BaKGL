#include "bak/textureFactory.hpp"

#include "bak/imageStore.hpp"
#include "bak/screen.hpp"

#include <algorithm>
#include <random>

namespace BAK {

Graphics::Texture ImageToTexture(const Image& image, const Palette& palette)
{
    auto texture = Graphics::Texture::TextureType{};
    const auto imageSize = image.GetWidth() * image.GetHeight();
    texture.reserve(imageSize);

    auto* pixels = image.GetPixels();

    for (unsigned i = 0; i < imageSize; i++)
    {
        texture.push_back(palette.GetColor(pixels[i]));
    }

    auto tex = Graphics::Texture{
        texture,
        static_cast<unsigned>(image.GetWidth()),
        static_cast<unsigned>(image.GetHeight()) };

    // For OpenGL
    tex.Invert();

    return tex;
}

Graphics::TextureStore TextureFactory::MakeTextureStore(
    std::string_view bmx,
    std::string_view pal)
{
    auto store = Graphics::TextureStore{};
    AddToTextureStore(store, bmx, pal);
    return store;
}

void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    std::string_view bmx,
    std::string_view pal)
{
    const auto palette = Palette{std::string{pal}};

    auto fb = FileBufferFactory::Get()
        .CreateDataBuffer(std::string{bmx});
    const auto images = LoadImages(fb);

    AddToTextureStore(store, images, palette);
}

void TextureFactory::AddScreenToTextureStore(
    Graphics::TextureStore& store,
    std::string_view scx,
    std::string_view pal)
{
    const auto palette = Palette{std::string{pal}};
    auto fb = FileBufferFactory::Get()
        .CreateDataBuffer(std::string{scx});
    AddToTextureStore(store, LoadScreenResource(fb), palette);
}

void TextureFactory::AddTerrainToTextureStore(
    Graphics::TextureStore& store,
    const Image& terrain,
    const Palette& palette)
{
    auto* pixels = terrain.GetPixels();
    auto width = terrain.GetWidth();

    auto startOff = 0;
    // FIXME: Can I find these in the data files somewhere?
    for (auto offset : {70, 20, 20, 32, 20, 27, 6, 5})
    {
        auto image = Graphics::Texture::TextureType{};
        const auto imageStart = startOff * width;
        const auto imageEnd = (startOff + offset) * width;
        image.reserve(imageEnd - imageStart);
        for (unsigned i = imageStart; i < imageEnd; i++)
        {
            const auto& color = palette.GetColor(pixels[i]);
            image.push_back(color);
        }
        if (offset == 70)
        {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(image.begin(), image.end(), g);
        }

        startOff += offset;
        
        store.AddTexture(
            Graphics::Texture{
                image,
                static_cast<unsigned>(width),
                static_cast<unsigned>(offset)});
    }
}

void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    const Image& image,
    const Palette& palette)
{
    store.AddTexture(ImageToTexture(image, palette));
}

void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    const std::vector<Image>& images,
    const Palette& palette)
{
    for (const auto& image : images)
        AddToTextureStore(store, image, palette);
}

} // namespace BAK {
