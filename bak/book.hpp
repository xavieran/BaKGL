#pragma once

#include "bak/file/fileBuffer.hpp"
#include <glm/glm.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace BAK {

class FileBuffer;

// Courtesy of Joris van Eijden
struct ReservedArea
{
    glm::ivec2 mPos;
    glm::ivec2 mDims;
};

enum class TextAlignment
{
    Left = 1,
    Right = 2,
    Justify = 3,
    Center = 4
};

enum class FontStyle
{
    Normal = 1,
    Bold = 2,
    Italic = 4,
    Underlined = 8,
    Ghosted = 16
};

struct TextSegment
{
    unsigned mFont;
    unsigned mYOff;
    FontStyle mFontStyle;
    std::string mText;
    unsigned mColor;
};

struct Paragraph
{
    glm::ivec2 mOffset;
    unsigned mWidth;
    unsigned mLineSpacing;
    unsigned mWordSpacing;
    unsigned mStartIndent;
    TextAlignment mAlignment;
    std::vector<TextSegment> mText;
};

enum class Mirroring
{
    None = 0,
    Horizontal = 1,
    Vertical = 2,
    HorizontalAndVertical = 3
};

struct BookImage
{
    glm::ivec2 mPos;
    unsigned mImage;
    Mirroring mMirroring;
};

struct Page
{
    // Offset applies to all text within this page
    unsigned mDisplayNumber;
    glm::ivec2 mOffset;
    glm::ivec2 mDims;

    unsigned mPageNumber;
    unsigned mPreviousPageNumber;
    unsigned mNextPageNumber;

    unsigned mShowPageNumber;

    std::vector<ReservedArea> mReservedAreas;
    std::vector<BookImage> mImages;
    std::vector<Paragraph> mParagraphs;
};

struct Book
{
    std::vector<Page> mPages;
};

Book LoadBook(FileBuffer&);
std::ostream& operator<<(std::ostream&, const Paragraph&);
std::ostream& operator<<(std::ostream&, const Page&);
std::ostream& operator<<(std::ostream&, const Book&);
}
