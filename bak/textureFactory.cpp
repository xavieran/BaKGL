#include "bak/textureFactory.hpp"

#include "bak/coordinates.hpp"

namespace BAK {

Graphics::Texture ImageToTexture(const Image& image, const BAK::Palette& palette)
{
    auto texture = Graphics::Texture::TextureType{};
    const auto imageSize = image.GetWidth() * image.GetHeight();
    texture.reserve(imageSize);

    auto* pixels = image.GetPixels();

    for (int i = 0; i < imageSize; i++)
    {
        texture.push_back(palette.GetColor(pixels[i]));
    }

    auto tex = Graphics::Texture{
        texture,
        static_cast<unsigned>(image.GetWidth()),
        static_cast<unsigned>(image.GetHeight())};

    // For OpenGL
    tex.Invert();
    
    return tex;
}

Graphics::TextureStore TextureFactory::MakeTextureStore(
    const ImageResource& images,
    const BAK::Palette& palette)
{
    auto store = Graphics::TextureStore{};
    for (unsigned i = 0; i < images.GetNumImages(); i++)
    {
        store.AddTexture(
            ImageToTexture(images.GetImage(i), palette));
    }

    return store;
}

Graphics::TextureStore TextureFactory::MakeTextureStore(
    std::string_view bmx,
    std::string_view pal)
{
    const auto palette = BAK::Palette{std::string{pal}};
    ImageResource images;
    {
        auto fb = FileBufferFactory::CreateFileBuffer(
            std::string{bmx});
        images.Load(&fb);
    }

    return MakeTextureStore(images, palette);
}

void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    const ImageResource& images,
    const BAK::Palette& palette)
{
    for (unsigned i = 0; i < images.GetNumImages(); i++)
    {
        store.AddTexture(
            ImageToTexture(images.GetImage(i), palette));
    }
}

void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    std::string_view bmx,
    std::string_view pal)
{
    const auto palette = BAK::Palette{std::string{pal}};

    ImageResource images;
    {
        auto fb = FileBufferFactory::CreateFileBuffer(
            std::string{bmx});
        images.Load(&fb);
    }

    AddToTextureStore(
        store,
        images,
        palette);
}


void TextureFactory::AddToTextureStore(
    Graphics::TextureStore& store,
    const ScreenResource& screen,
    const BAK::Palette& palette)
{
    store.AddTexture(
        ImageToTexture(screen.GetImage(), palette));
}

void TextureFactory::AddScreenToTextureStore(
    Graphics::TextureStore& store,
    std::string_view scx,
    std::string_view pal)
{
    const auto palette = BAK::Palette{std::string{pal}};
    ScreenResource screen;
    {
        auto fb = FileBufferFactory::CreateFileBuffer(
            std::string{scx});
        screen.Load(&fb);
    }

    AddToTextureStore(
        store,
        screen,
        palette);
}

void TextureFactory::AddTerrainToTextureStore(
    Graphics::TextureStore& store,
    const ScreenResource& terrain,
    const BAK::Palette& palette)
{
    auto* pixels = terrain.GetImage()->GetPixels();
    auto width = terrain.GetImage()->GetWidth();

    auto startOff = 0;
    // FIXME: Can I find these in the data files somewhere?
    for (auto offset : {70, 20, 20, 32, 20, 27, 6, 5})
    {
        auto image = Graphics::Texture::TextureType{};
        const auto imageStart = startOff * width;
        const auto imageEnd = (startOff + offset) * width;
        image.reserve(imageEnd - imageStart);
        for (int i = imageStart; i < imageEnd; i++)
        {
            const auto& color = palette.GetColor(pixels[i]);
            image.push_back(color);
        }
        if (offset == 70)
            std::random_shuffle(image.begin(), image.end());

        startOff += offset;
        
        store.AddTexture(
            Graphics::Texture{
                image,
                static_cast<unsigned>(width),
                static_cast<unsigned>(offset)});
    }
}

} // namespace BAK {
