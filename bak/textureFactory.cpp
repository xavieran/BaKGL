#include "bak/textureFactory.hpp"

#include "bak/image.hpp"
#include "bak/imageStore.hpp"
#include "bak/palette.hpp"
#include "bak/screen.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/png.hpp"
#include "com/path.hpp"
#include "com/string.hpp"

#include "graphics/texture.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
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
        static_cast<unsigned>(image.GetHeight()),
        static_cast<unsigned>(image.GetWidth()),
        static_cast<unsigned>(image.GetHeight()) };

    // For OpenGL
    tex.Invert();

    return tex;
}

Graphics::Texture PNGToTexture(std::string path, unsigned targetWidth, unsigned targetHeight)
{
    const auto image = LoadPNG(path.c_str());
    const auto width = image.mWidth;
    const auto height = image.mHeight;
    const auto Get = [&](int x, int y){
        return image.mPixels[y * width + x];
    };

    auto texture = Graphics::Texture::TextureType{};

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < (int) width; x++)
        {
            auto c = Get(x, y);
            const auto F = [](auto x){
                return static_cast<float>(x) / 255.; };
            texture.push_back(glm::vec4{F(c.r), F(c.g), F(c.b), F(c.a)});
        }
    }
    return Graphics::Texture{texture, width, height, targetWidth, targetHeight};
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

    auto baseName = SplitString(".", std::string(bmx))[0];
    auto substitute = images.size() > 1
        ? Paths::Get().GetModDirectoryPath() / (baseName + ".BMX")
        : Paths::Get().GetModDirectoryPath() / (baseName + ".PNG");
    if (std::filesystem::exists(substitute) && images.size() == 1)
    {
        auto tex = PNGToTexture(substitute.string(), images.back().GetWidth(), images.back().GetHeight());
        Logging::LogDebug(__FUNCTION__) << "Found substitute BMX: " << substitute
          << " Dims: (" << tex.GetWidth() << ", " << tex.GetHeight() << ") TargetDims: ("
          << tex.GetTargetWidth() << ", " << tex.GetTargetHeight() << ")\n";
        store.AddTexture(tex);
    }
    else if (std::filesystem::exists(substitute))
    {
        for (unsigned i = 0; i < images.size(); i++)
        {
            std::stringstream name{};
            name << i << ".PNG";
            auto path = substitute / name.str();
            if (std::filesystem::exists(path))
            {
                auto tex = PNGToTexture(path.string(), images[i].GetWidth(), images[i].GetHeight());
                Logging::LogDebug(__FUNCTION__) << "Found substitute BMX: " << path 
                  << " Dims: (" << tex.GetWidth() << ", " << tex.GetHeight() << ") TargetDims: ("
                  << tex.GetTargetWidth() << ", " << tex.GetTargetHeight() << ")\n";
                store.AddTexture(tex);
            }
            else
            {
                AddToTextureStore(store, images[i], palette);
            }
        }
    }
    else
    {
        AddToTextureStore(store, images, palette);
    }
}

void TextureFactory::AddScreenToTextureStore(
    Graphics::TextureStore& store,
    std::string_view scx,
    std::string_view pal)
{
    auto baseName = SplitString(".", std::string(scx))[0];
    auto substitute = Paths::Get().GetModDirectoryPath() / (baseName + ".PNG");

    if (std::filesystem::exists(substitute))
    {
        auto fb = FileBufferFactory::Get()
            .CreateDataBuffer(std::string{scx});
        auto target = LoadScreenResource(fb);
        Logging::LogDebug(__FUNCTION__) << "Found substitute SCX: " << substitute << "\n";
        store.AddTexture(PNGToTexture(substitute.string(), target.GetWidth(), target.GetHeight()));
    }
    else
    {
        const auto palette = Palette{std::string{pal}};
        auto fb = FileBufferFactory::Get()
            .CreateDataBuffer(std::string{scx});
        AddToTextureStore(store, LoadScreenResource(fb), palette);
    }
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
                static_cast<unsigned>(offset),
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
