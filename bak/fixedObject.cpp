#include "bak/fixedObject.hpp"

namespace BAK {

FixedObject::FixedObject(
    Target dialogKey,
    glm::vec<2, unsigned> location,
    std::uint8_t type,
    std::optional<HotspotRef> hotspotRef,
    std::optional<TeleportIndex> teleport)
:
    mDialogKey{dialogKey},
    mLocation{location},
    mType{type},
    mHotspotRef{hotspotRef},
    mTeleport{teleport}
{}

std::ostream& operator<<(std::ostream& os, const FixedObject& obj)
{
    os << "FixedObject {" << obj.mDialogKey << " loc: " << obj.mLocation << " type: " << +obj.mType << " hs: " << obj.mHotspotRef << "}";
    return os;
}

std::vector<FixedObject> LoadFixedObjects(
    unsigned targetZone)
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
            std::optional<TeleportIndex> teleport{};
            auto hotspotRef = std::optional<HotspotRef>{};

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
                teleport = TeleportIndex{fb.GetUint16LE()};
                dialogKey = fb.GetUint32LE();
                fb.DumpAndSkip(3);
            }
            else if (type == 0xa)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
                fb.DumpAndSkip(4);
                hotspotRef = HotspotRef{
                    fb.GetUint8(),
                    static_cast<char>(
                        fb.GetUint8() + 0x40)};
                if (hotspotRef == HotspotRef{0, 0x40})
                    hotspotRef.reset();
                fb.DumpAndSkip(3);
            }

            logger.Debug() << "(" << x << "," << y << ") zone: " << +zone
                << " tp: " << +type << " dialog: " << std::hex
                << dialogKey << " hs: " << hotspotRef << std::dec 
                << " tele: " << teleport << std::endl;

            if (targetZone == zone)
            {
                fixedObjects.emplace_back(
                    KeyTarget{dialogKey},
                    glm::vec<2, unsigned>{x, y},
                    type,
                    hotspotRef,
                    teleport);
            }
        }
    }

    return fixedObjects;
}

}
