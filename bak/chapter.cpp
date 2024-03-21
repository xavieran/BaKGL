#include "bak/chapter.hpp"

#include "bak/coordinates.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace BAK {

void LoadChapter(Chapter chapter)
{
    std::stringstream ss{};
    ss << "CHAP";
    ss << chapter.mValue;
    ss << ".DAT";

    auto fb = FileBufferFactory::Get().CreateDataBuffer(ss.str());

    unsigned fileChapter = fb.GetUint16LE();
    fb.Skip(4);
    unsigned unknown0 = fb.GetUint8();
    unsigned unknown1 = fb.GetUint8();
    fb.Skip(8);
    unsigned zone = fb.GetUint8();
    unsigned tileX = fb.GetUint8();
    unsigned tileY = fb.GetUint8();
    unsigned cellX = fb.GetUint8();
    unsigned cellY = fb.GetUint8();
    unsigned heading = fb.GetUint16LE();

    auto pos = MakeGamePositionFromTileAndOffset(glm::uvec2{tileX, tileY}, glm::uvec2{cellX, cellY});
    Logging::LogDebug(__FUNCTION__) << "Chapter: " << fileChapter
        << " unknowns: [" << unknown0 << " " << unknown1 << "]"
        << " zone: " << zone << " tileX: " << tileX << " tileY: "
        << tileY << " cellX: " << cellX << " cellY: " << cellY
        << " heading: " << heading << "\n";
    Logging::LogDebug(__FUNCTION__) << "StartPos: " << pos << "\n";
}

}
