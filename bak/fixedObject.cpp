#include "bak/fixedObject.hpp"

#include "bak/coordinates.hpp"

#include "com/ostream.hpp"

namespace BAK {


std::vector<GenericContainer> LoadFixedObjects(
    unsigned targetZone)
{
    std::vector<GenericContainer> fixedObjects;

    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    auto fb = FileBufferFactory::CreateFileBuffer("OBJFIXED.DAT");

    fb.DumpAndSkip(2);

    unsigned z = 1;
    while (fb.GetBytesLeft() > 0)
    {
        logger.Debug() << "Z: " << z++ << " @" << std::hex << fb.Tell() << std::dec << std::endl;
        auto objects = fb.GetUint16LE();
        logger.Debug() << "Objects: " << objects << std::endl;
        for (unsigned i = 0; i < objects; i++)
        {
            auto container = LoadGenericContainer(fb, false);
            logger.Debug() << "Obj no: " << i << std::endl;
            logger.Debug() << container << std::endl;
            if (container.mHeader.GetZone().mValue == targetZone)
                fixedObjects.emplace_back(std::move(container));
        }
    }

    return fixedObjects;
}



}
