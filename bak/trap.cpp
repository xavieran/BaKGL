#include "bak/trap.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "graphics/glm.hpp"

namespace BAK {

std::string_view ToString(TrapElementType e)
{
    using enum TrapElementType;
    switch (e)
    {
    case RedCrystal: return "RedCrystal";
    case GreenCrystal: return "GreenCrystal";
    case SolidDiamond: return "SolidDiamond";
    case TransparentDiamond: return "TransparentDiamond";
    case Unknown_C3: return "Unknown_C3";
    case Unknown_FFEE: return "Unknown_FFEE";
    case Character0: return "Character0";
    case Character1: return "Character1";
    case Character2: return "Character2";
    case BlasterDown: return "BlasterDown";
    case BlasterUp: return "BlasterUp";
    case BlasterRight: return "BlasterRight";
    case BlasterLeft: return "BlasterLeft";
    case Exit: return "Exit";
    default: return "UnknownElement";
    }
}

std::ostream& operator<<(std::ostream& os, const Trap& trap)
{
    os << "Trap #" << trap.mIndex << "\n";
    for (const auto& elem : trap.mElements)
    {
        os << "  " << ToString(elem.mElement) << " - " << elem.mPosition << "\n";
    }
    return os;
}

std::vector<Trap> LoadTraps()
{
    std::vector<Trap> traps{};

    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    auto fb = FileBufferFactory::Get().CreateDataBuffer("TRAPS.DAT");

    // Zun Trap starts at 0x3a2 -- Trap #15
    unsigned i = 0;
    while (fb.GetBytesLeft() != 0)
    {
        auto trap = Trap{};
        trap.mIndex = i++;
        auto numElements = fb.GetUint16LE();
        logger.Debug() << "@" << std::hex << fb.Tell() << std::dec << "\n";
        for (unsigned j = 0; j < numElements; j++)
        {
            auto elementType = fb.GetUint16LE();
            auto pos = fb.LoadVector<std::uint8_t, 2>();
            trap.mElements.emplace_back(TrapElement{static_cast<TrapElementType>(elementType), pos});
        }
        for (unsigned j = 0; j < (15u - numElements); j++)
        {
            fb.Skip(4);
        }
        logger.Debug() << trap << "\n";

        traps.emplace_back(trap);
    }

    return traps;
}

}
