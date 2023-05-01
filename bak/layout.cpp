#include "bak/layout.hpp"

#include "com/assert.hpp"

#include "bak/fileBufferFactory.hpp"

namespace BAK {

Layout::Layout(
    const std::string& path)
:
    mIsPopup{false},
    mPosition{},
    mDimensions{},
    mOffset{},
    mWidgets{}
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(path);

    fb.Skip(2);
    mIsPopup = (fb.GetSint16LE() != 0);
    fb.Skip(2);

    const auto xPos = fb.GetSint16LE();
    const auto yPos = fb.GetSint16LE();
    mPosition = glm::vec2{xPos, yPos};

    const auto width = fb.GetSint16LE();
    const auto height = fb.GetSint16LE();
    mDimensions = glm::vec2{width, height};

    fb.Skip(2);

    const auto xOff = fb.GetSint16LE();
    const auto yOff = fb.GetSint16LE();
    mOffset = glm::vec2{xOff, yOff};

    fb.Skip(8);

    const auto numWidgets = fb.GetUint16LE();
    std::vector<int> stringOffsets;
    stringOffsets.reserve(numWidgets);

    for (unsigned i = 0; i < numWidgets; i++)
    {
        auto pos = fb.GetBytesDone();
        const auto widget = fb.GetUint16LE();
        const auto action = fb.GetSint16LE();
        const auto isVisible = fb.GetUint8() > 0;
        fb.Skip(2);
        fb.Skip(2);
        fb.Skip(2);
        const auto xPos = fb.GetSint16LE();
        const auto yPos = fb.GetSint16LE();
        const auto width = fb.GetUint16LE();
        const auto height = fb.GetUint16LE();
        fb.Skip(2);
        stringOffsets.emplace_back(fb.GetSint16LE());
        const auto teleport = fb.GetSint16LE();
        const auto image = fb.GetUint16LE();
        fb.Skip(2);
        const auto group = fb.GetUint16LE();
        fb.Skip(2);

        mWidgets.emplace_back(
            widget,
            action,
            isVisible,
            glm::vec2{xPos, yPos},
            glm::vec2{width, height},
            teleport,
            (image >> 1) + (image & 1),
            group,
            "");
    }
    
    fb.Skip(2);
    const auto stringStart = fb.GetBytesDone();
    for (unsigned i = 0; i < numWidgets; i++)
    {
        if (stringOffsets[i] >= 0)
        {
            fb.Seek(stringStart + stringOffsets[i]);
            mWidgets[i].mLabel = fb.GetString();
        }
    }
}

const Layout::LayoutWidget& Layout::GetWidget(unsigned index) const
{
    ASSERT(index < mWidgets.size());
    return mWidgets[index];
}

glm::vec2 Layout::GetWidgetLocation(unsigned index) const
{
    return mPosition + mOffset + GetWidget(index).mPosition;
}

glm::vec2 Layout::GetWidgetDimensions(unsigned index) const
{
    return GetWidget(index).mDimensions;
}

}
