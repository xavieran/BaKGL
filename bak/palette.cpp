#include "bak/palette.hpp"

#include "bak/dataTags.hpp"
#include "bak/fileBufferFactory.hpp"


#include "com/assert.hpp"

#include <functional>

namespace BAK {

Palette::Palette(const std::string& filename)
:
    mColors{}
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(filename);
    *this = Palette{fb};
}

Palette::Palette(FileBuffer& fb)
:
    mColors{}
{
    auto palbuf = fb.Find(DataTag::VGA);
    const auto size = palbuf.GetSize() / 3;
    mColors.reserve(size);
    mColors8.reserve(size);

    const auto F = [](auto x){
        return static_cast<float>(x) / 255.; };

    for (unsigned i = 0; i < size; i++)
    {
        const std::uint8_t r = palbuf.GetUint8() << 2;
        const std::uint8_t g = palbuf.GetUint8() << 2;
        const std::uint8_t b = palbuf.GetUint8() << 2;
        const auto a = i == 0 ? 0.0 : 1.0;
        mColors.emplace_back(F(r), F(g), F(b) , a);
        std::uint8_t alpha = i == 0 ? 0 : 255;
        mColors8.emplace_back(std::array<std::uint8_t, 4>{r, g, b, alpha});
    }
}

Palette::Palette(const Palette& pal, const ColorSwap& cs)
:
    mColors{std::invoke([&](){
        auto swappedPal = std::vector<glm::vec4>{};
        for (unsigned i = 0; i < 256; i++)
        {
            swappedPal.emplace_back(cs.GetColor(i, pal));
        }
        return swappedPal;
    })}
{
}

const glm::vec4& Palette::GetColor(unsigned i) const
{
    ASSERT(i < mColors.size());
    return mColors[i];
}

const std::vector<std::array<std::uint8_t, 4>>& Palette::GetColors8() const
{
    return mColors8;
}

ColorSwap::ColorSwap(const std::string& filename)
:
    mIndices{}
{
    mIndices.reserve(sSize);
    auto fb = FileBufferFactory::Get().CreateDataBuffer(filename);

    for (unsigned i = 0; i < sSize; i++)
    {
        mIndices.emplace_back(fb.GetUint8());
    }
}

const glm::vec4& ColorSwap::GetColor(unsigned i, const Palette& pal) const
{
    ASSERT(i < sSize);
    return pal.GetColor(mIndices[i]);
}

}
