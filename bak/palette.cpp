#include "bak/palette.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/TaggedResource.h"

namespace BAK {

Palette::Palette(const std::string& filename)
:
    mColors{}
{
    auto fb = FileBufferFactory::CreateFileBuffer(filename);
    auto palbuf = fb.Find(TAG_VGA);
    const auto size = palbuf.GetSize() / 3;
    mColors.reserve(size);

    const auto F = [](auto x){
        return static_cast<float>(x) / 256.; };

    for (unsigned i = 0; i < size; i++)
    {
        const auto r = F(palbuf.GetUint8() << 2);
        const auto g = F(palbuf.GetUint8() << 2);
        const auto b = F(palbuf.GetUint8() << 2);
        const auto a = i == 0 ? 0.0 : 1.0;
        mColors.emplace_back(r, g, b , a);
    }
}

const glm::vec4& Palette::GetColor(unsigned i) const
{
    assert(i < mColors.size());
    return mColors[i];
}

}
