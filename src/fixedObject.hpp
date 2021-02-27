#pragma once

#include "dialog.hpp"
#include "logger.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace BAK {

class FixedObject
{
public:
    Target mDialogKey;
    glm::vec<2, unsigned> mLocation;
};

std::vector<FixedObject> LoadFixedObjects(unsigned targetZone)
{
    std::vector<FixedObject> fixedObjects;

    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    auto fb = FileBufferFactory::CreateFileBuffer("OBJFIXED.DAT");

    fb.DumpAndSkip(2);

    unsigned z = 1;
    while (fb.GetBytesLeft() > 0)
    {
        logger.Debug() << "Z: " << z++ << " @" << std::hex << fb.Tell() << std::dec << std::endl;
        auto objects = fb.GetUint16LE();
        logger.Debug() << "Objects: " << objects<< std::endl;
        for (unsigned i = 0; i < objects; i++)
        {
            logger.Debug() << "Obj no: " << i << std::endl;
            auto zone = fb.GetUint8();

            fb.DumpAndSkip(3);

            auto x = fb.GetUint32LE();
            auto y = fb.GetUint32LE();

            fb.DumpAndSkip(3);

            auto type = fb.GetUint8();
            std::uint32_t dialogKey = 0;

            if (type == 0x0)
            {
                // No dialog
            }
            else if (type == 0x2
                || type == 0x21)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
            }
            else if (type == 0x8)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
                fb.DumpAndSkip(3);
            }
            else if (type == 0xa)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
                fb.DumpAndSkip(9);
            }
            logger.Debug() << "(" << x << "," << y << ") zone: " << +zone
                << " tp: " << +type << " dialog: " << std::hex
                << dialogKey << std::dec << std::endl;

            if (targetZone == zone)
            {
                fixedObjects.emplace_back(
                    KeyTarget{dialogKey},
                    glm::vec<2, unsigned>{x, y});
            }
        }
    }

    return fixedObjects;
}

}
