#include "bak/book.hpp"

#include "bak/file/fileBuffer.hpp"

#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <optional>
#include <string_view>

namespace BAK {

std::string_view ToString(TextAlignment t)
{
    using enum TextAlignment;
    switch (t)
    {
        case Left: return "Left";
        case Right: return "Right";
        case Justify: return "Justify";
        case Center: return "Center";
    }
    return "UnknownTextAlignment";
}

std::string_view ToString(FontStyle f)
{
    using enum FontStyle;
    switch (f)
    {
        case Normal: return "Normal";
        case Bold: return "Bold";
        case Italic: return "Italic";
        case Underlined: return "Underlined";
        case Ghosted: return "Ghosted";
    }
    return "UnknownFontStyle";
}

std::string ToString(Mirroring m)
{
    using enum Mirroring;
    switch (m)
    {
        case None: return "None";
        case Horizontal: return "Horizontal";
        case Vertical: return "Vertical";
        case HorizontalAndVertical: return "HorizontalAndVertical";
    }
    return "UnknownMirroring" + std::to_string(static_cast<std::uint16_t>(m));
}

std::ostream& operator<<(std::ostream& os, const ReservedArea& r)
{
    os << "    Reserved { pos: " << r.mPos << " dims: " << r.mDims << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BookImage& i)
{
    os << "    BookImage { pos: " << i.mPos << " img: " << i.mImage << " "
        << ToString(i.mMirroring) << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TextSegment& p)
{
    os << "      TextSegment { font: " << p.mFont << " yoff: " << p.mYOff
        << " fs: " << ToString(p.mFontStyle) << " color: " << p.mColor << "\n";
    os << "      " << p.mText << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Paragraph& p)
{
    os << "    Paragraph { off: " << p.mOffset << " width: " << p.mWidth
        << " lineSpc: " << p.mLineSpacing << " wordSpc: " << p.mWordSpacing
        << " indent: " << p.mStartIndent << " align: " << ToString(p.mAlignment)
        << " text:\n";
    for (const auto& ts : p.mText)
    {
        os << ts;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Page& page)
{
    os << "Page\n" "  details: " << page.mDisplayNumber << " off: " << page.mOffset
        << " dims: " << page.mDims << " pg: " << page.mPageNumber
        << " prevPg: " << page.mPreviousPageNumber << " nextPg: "
        << page.mNextPageNumber << " showPg: " << page.mShowPageNumber
        << "\n";
    os << "  Reserved areas: \n";
    for (const auto& reserved : page.mReservedAreas)
    {
        os << reserved << "\n";
    }

    os << "  Images: \n";
    for (const auto& image : page.mImages)
    {
        os << image << "\n";
    }
    os << "  Paragraphs: \n";
    for (const auto& paragraph : page.mParagraphs)
    {
        os << paragraph << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Book& book)
{
    os << "Book:";
    for (const auto& page : book.mPages)
    {
        os << page;
    }
    return os;
}

Page LoadPage(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    Page page;

    logger.Debug() << "Location: " << fb.Tell() << "\n";
    unsigned posX = fb.GetSint16LE();
    unsigned posY = fb.GetSint16LE();
    page.mOffset = glm::ivec2{posX, posY};

    unsigned dimX = fb.GetSint16LE();
    unsigned dimY = fb.GetSint16LE();
    page.mDims = glm::ivec2{dimX, dimY};

    page.mDisplayNumber = fb.GetUint16LE();
    page.mPageNumber = fb.GetUint16LE();
    page.mPreviousPageNumber = fb.GetUint16LE();
    page.mNextPageNumber = fb.GetUint16LE();
    unsigned pagePointer = fb.GetUint16LE();
    fb.Skip(2);

    unsigned images = fb.GetUint16LE();
    unsigned reserveds = fb.GetUint16LE();
    page.mShowPageNumber = fb.GetUint16LE();

    fb.DumpAndSkip(4);
    fb.DumpAndSkip(16);
    fb.DumpAndSkip(10);

    for (unsigned i = 0 ; i < reserveds; i++)
    {
        auto x = fb.GetSint16LE();
        auto y = fb.GetSint16LE();
        auto width = fb.GetSint16LE();
        auto height = fb.GetSint16LE();
        page.mReservedAreas.emplace_back(ReservedArea{{x, y}, {width, height}});
    }

    for (unsigned i = 0 ; i < images; i++)
    {
        auto x = fb.GetSint16LE();
        auto y = fb.GetSint16LE();
        unsigned image = fb.GetUint16LE();
        Mirroring mirror = static_cast<Mirroring>(fb.GetUint16LE());
        page.mImages.emplace_back(BookImage{{x, y}, image, mirror});
    }

    static constexpr std::uint8_t EndPage = 0xf0;
    static constexpr std::uint8_t StartParagraph = 0xf1;
    static constexpr std::uint8_t StartText = 0xf4;
    static constexpr std::uint8_t UpperCharacterLimit = 0xb1;

    bool pageDone = false;
    auto paragraph = std::optional<Paragraph>();
    while (!pageDone)
    {
        const auto delimiter = fb.GetUint8();
        if (delimiter == EndPage)
        {
            pageDone = true;
            if (paragraph)
            {
                page.mParagraphs.emplace_back(*paragraph);
            }
        }
        else if (delimiter == StartParagraph)
        {
            if (paragraph)
            {
                page.mParagraphs.emplace_back(*paragraph);
            }
            paragraph = Paragraph{};
            const auto xoff = fb.GetSint16LE();
            paragraph->mWidth = fb.GetSint16LE();
            paragraph->mLineSpacing = fb.GetUint16LE();
            paragraph->mWordSpacing = fb.GetUint16LE();
            paragraph->mStartIndent = fb.GetUint16LE();
            fb.Skip(2);
            const auto yoff = fb.GetSint16LE();
            paragraph->mOffset = glm::ivec2{xoff, yoff};
            paragraph->mAlignment = static_cast<TextAlignment>(fb.GetUint16LE());
        }
        else if (delimiter == StartText)
        {
            auto text = TextSegment{};
            text.mFont = fb.GetUint16LE();
            text.mYOff = fb.GetSint16LE();
            text.mColor = fb.GetUint16LE();
            fb.Skip(2);
            text.mFontStyle = static_cast<FontStyle>(fb.GetUint16LE());
            std::stringstream ss{};
            while (fb.Peek() < UpperCharacterLimit)
            {
                ss << fb.GetUint8();
            }
            text.mText = ss.str();
            assert(paragraph);
            paragraph->mText.emplace_back(text);
        }
        else
        {
        }
    }
    
    return page;
}

Book LoadBook(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    unsigned size = fb.GetUint32LE();
    unsigned pageCount = fb.GetUint16LE();

    std::vector<unsigned> pageOffsets;

    for (unsigned i = 0; i < pageCount; i++)
    {
        pageOffsets.emplace_back(fb.GetUint32LE() + 4);
    }

    Book book{};
    for (unsigned i = 0; i < pageCount; i++)
    {
        fb.Seek(pageOffsets[i]);
        auto page = LoadPage(fb);
        book.mPages.emplace_back(page);
    }

    return book;
}

}
