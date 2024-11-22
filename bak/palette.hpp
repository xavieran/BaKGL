#pragma once

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>

namespace BAK {

class FileBuffer;
class Palette;

class ColorSwap
{
public:
    static constexpr auto sSize = 256;

    ColorSwap(const std::string& filename);

    const glm::vec4& GetColor(unsigned i, const Palette&) const;

private:
    std::vector<unsigned> mIndices;
};

class Palette
{
public:
    Palette(const std::string& filename);
    Palette(FileBuffer& fb);
    Palette(const Palette& pal, const ColorSwap& cs);

    const glm::vec4& GetColor(unsigned i) const;
    const std::vector<std::array<std::uint8_t, 4>>& GetColors8() const;
private:
    std::vector<glm::vec4> mColors;
    std::vector<std::array<std::uint8_t, 4>> mColors8;
};


}
