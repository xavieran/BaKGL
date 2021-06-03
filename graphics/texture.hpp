#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "src/texture.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"
#include "xbak/ImageResource.h"
#include "xbak/PaletteResource.h"

namespace Graphics {

class Texture
{
public:

    Texture(
        std::string_view bmx,
        std::string_view pal)
    :
        mTextures{},
        mMaxHeight{0},
        mMaxWidth{0},
        mMaxDim{0}
    {
        unsigned textures = 0;
        bool found = true;

        PaletteResource palette{};
        palette.Load(FileManager::GetInstance()->LoadResource(std::string{pal}));

        ImageResource images;
        images.Load(FileManager::GetInstance()->LoadResource(std::string{bmx}));

        for (unsigned j = 0; j < images.GetNumImages(); j++)
        {
            assert(images.GetImage(j));
            const auto& image = *images.GetImage(j);

            mTextures.push_back(
                BAK::ImageToTexture(
                    image,
                    *palette.GetPalette()));

            textures++;
        }

        // Set max width and height
        mMaxHeight = std::max_element(
            mTextures.begin(), mTextures.end(),
            [](const auto &lhs, const auto& rhs){
            return (lhs.GetHeight() < rhs.GetHeight());
            })->GetHeight();

        mMaxWidth = std::max_element(
			mTextures.begin(), mTextures.end(),
            [](const auto &lhs, const auto& rhs){
            return (lhs.GetWidth() < rhs.GetWidth());
            })->GetWidth();

        mMaxDim = std::max(mMaxWidth, mMaxHeight);
    }

    const BAK::Texture& GetTexture(const unsigned i) const
    {
        assert(i < mTextures.size());
        return mTextures[i];
    }

    const std::vector<BAK::Texture>& GetTextures() const { return mTextures; }

    unsigned GetMaxDim() const { return mMaxDim; }
    unsigned GetMaxHeight() const { return mMaxHeight; }
    unsigned GetMaxWidth() const { return mMaxWidth; }

private:
    std::vector<BAK::Texture> mTextures;

    unsigned mMaxHeight;
    unsigned mMaxWidth;
    unsigned mMaxDim;
};

}
