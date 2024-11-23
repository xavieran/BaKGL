#include "bak/dataTags.hpp"
#include "bak/font.hpp"

#include "bak/file/fileBuffer.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/texture.hpp"

#include <iostream>
#include <vector>

namespace BAK {

Glyph::Glyph(
    unsigned width,
    unsigned height,
    std::array<std::uint16_t, MAX_FONT_HEIGHT> points)
:
    mWidth{width},
    mHeight{height},
    mPoints{points}
{}

Graphics::Texture GlyphToTexture(const Glyph& glyph)
{
    auto data = Graphics::Texture::TextureType{};
    for (int r = glyph.mHeight - 1; r >= 0; r--)
    {
        for (unsigned c = 0; c < glyph.mWidth; c++)
        {
            const auto pixel = (glyph.mPoints[r] & (0x8000 >> c))
                ? glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}
                : glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
            
            data.push_back(pixel);
        }
    }

    return Graphics::Texture{data, glyph.mWidth, glyph.mHeight, glyph.mWidth, glyph.mHeight};
}

Font::Font(
    int firstChar,
    unsigned height,
    Graphics::TextureStore textures)
:
    mFirstChar{firstChar},
    mHeight{height},
    mCharacters{textures}
{}

char Font::GetIndex(char c) const
{
    if (!(mFirstChar <= c))
    {
        Logging::LogError   ("BAK::Font") << "Request for bad char: {" 
            << std::hex << +c << std::dec << "} [" << c << "]" << std::endl;
        return mFirstChar;
    }
    ASSERT(mFirstChar <= c);
    return c - mFirstChar;
}

const Graphics::TextureStore& Font::GetCharacters(){ return mCharacters; }

std::size_t Font::GetSpace() const
{
    // The width of 'a'
    return static_cast<float>(
        mCharacters.GetTexture(0).GetWidth());
}

char Font::GetFirstChar() const { return mFirstChar; }
unsigned Font::GetWidth(char c) const
{
    return mCharacters.GetTexture(GetIndex(c)).GetWidth();
}

unsigned Font::GetHeight() const
{
    return mHeight;
}

Font LoadFont(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto fntBuf = fb.Find(DataTag::FNT);
    
    const auto version = fntBuf.GetUint8();
    const auto maxWidth = fntBuf.GetUint8();
    const auto height = fntBuf.GetUint8();
    const auto baseline = fntBuf.GetUint8();
    const auto firstChar = fntBuf.GetUint8();
    const auto numChars = fntBuf.GetUint8();
    const auto dataLength = fntBuf.GetUint16LE();

    if (fntBuf.GetUint8() != 0x01)
        throw std::runtime_error("Expected font to be RLE compressed");

    const auto decompSize = fntBuf.GetUint32LE();
    auto glyphBuf = FileBuffer(decompSize);

    fntBuf.DecompressRLE(&glyphBuf);
    
    auto glyphOffsets = std::vector<unsigned>{};
    for (auto i = 0; i < numChars; i++)
        glyphOffsets.emplace_back(glyphBuf.GetUint16LE());

    const auto glyphDataStart = glyphBuf.GetBytesDone();

    auto glyphs = std::vector<Glyph>{};

    auto textures = Graphics::TextureStore{}; 

    for (auto i = 0; i < numChars; i++)
    {
        glyphBuf.Seek(glyphDataStart + i);
        const auto width = glyphBuf.GetUint8();
        glyphBuf.Seek(glyphDataStart + numChars + glyphOffsets[i]);

        if (version == 0xff)
        {
            std::array<std::uint16_t, 16> points{};
            for (auto j = 0; j < height; j++)
            {
                points[j] = static_cast<std::uint16_t>(glyphBuf.GetUint8()) << 8;
                if (width > 8)
                    points[j] |= static_cast<std::uint16_t>(glyphBuf.GetUint8());
            }
            textures.AddTexture(GlyphToTexture(Glyph(width, height, points)));
        }
        // SPELL.FNT
        else if (version == 0xfd)
        {
            auto data = Graphics::Texture::TextureType{};
            for (unsigned r = 0; r < height; r++)
            {
                for (unsigned c = 0; c < width; c++)
                {
                    auto index = glyphBuf.GetUint8();
                    // FIXME: probably do this elsewhere...
                    float shade = static_cast<float>(index) / 128.0f;
                    auto color = glm::vec4{shade, 0, 0, index != 0 ? 1.0f : 0.0f};
                    data.push_back(color);
                }
            }
            auto texture = Graphics::Texture{data, width, height, width, height};
            texture.Invert();
            textures.AddTexture(texture);
        }
        else
        {
            throw std::runtime_error("Unexpected font version: ");
        }
    }
    // OpenGL requires min texture dims of 16
    auto empty = Glyph{16, 16, std::array<std::uint16_t, 16>{}};
    textures.AddTexture(GlyphToTexture(empty));

    return Font{
        firstChar,
        height,
        textures};
}

}
