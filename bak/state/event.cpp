#include "bak/state/event.hpp"

#include "bak/state/offsets.hpp"

#include "com/bits.hpp"
#include "com/logger.hpp"

namespace BAK::State {

std::pair<unsigned, unsigned> CalculateComplexEventOffset(unsigned eventPtr) 
{
    const auto source = (eventPtr + 0x2540) & 0xffff;
    const auto byteOffset = source / 10;
    const auto bitOffset = source % 10 != 0
        ? (source % 10) - 1
        : 0;
        
    Logging::LogSpam(__FUNCTION__) << std::hex << " " << eventPtr << " source: " << source << " ("
        << byteOffset + sGameComplexEventRecordOffset << ", " 
        << bitOffset << ")\n" << std::dec;
    return std::make_pair(
        byteOffset + sGameComplexEventRecordOffset,
        bitOffset);
}

std::pair<unsigned, unsigned> CalculateEventOffset(unsigned eventPtr) 
{
    const unsigned startOffset = sGameEventRecordOffset;
    const unsigned bitOffset = eventPtr & 0xf;
    const unsigned byteOffset = (0xfffe & (eventPtr >> 3)) + startOffset;
    Logging::LogSpam(__FUNCTION__) << std::hex << " " << eventPtr << " ("
        << byteOffset << ", " << bitOffset << ")\n" << std::dec;
    return std::make_pair(byteOffset, bitOffset);
}

void SetBitValueAt(FileBuffer& fb, unsigned byteOffset, unsigned bitOffset, unsigned value)
{
    fb.Seek(byteOffset);
    const auto originalData = fb.GetUint16LE();
    const auto data = SetBit(originalData, bitOffset, value != 0);

    fb.Seek(byteOffset);
    fb.PutUint16LE(data);

    Logging::LogSpam(__FUNCTION__) << std::hex << 
        " " << byteOffset << " " << bitOffset 
        << " original[" << +originalData << "] new[" << +data  <<"]\n" << std::dec;
}

void SetEventFlag(FileBuffer& fb, unsigned eventPtr, unsigned value)
{
    Logging::LogSpam(__FUNCTION__) << " " << std::hex << eventPtr 
        << " to: " << value << std::dec << "\n";
    if (eventPtr >= 0xdac0)
    {
        const auto [byteOffset, bitOffset] = CalculateComplexEventOffset(eventPtr);
        SetBitValueAt(fb, byteOffset, bitOffset, value);
    }
    else
    {
        const auto [byteOffset, bitOffset] = CalculateEventOffset(eventPtr);
        SetBitValueAt(fb, byteOffset, bitOffset, value);
    }
}

void SetEventFlagTrue (FileBuffer& fb, unsigned eventPtr)
{
    SetEventFlag(fb, eventPtr, 1);
}

void SetEventFlagFalse(FileBuffer& fb, unsigned eventPtr)
{
    SetEventFlag(fb, eventPtr, 0);
}

unsigned ReadBitValueAt(FileBuffer& fb, unsigned byteOffset, unsigned bitOffset) 
{
    fb.Seek(byteOffset);
    const unsigned eventData = fb.GetUint16LE();
    const unsigned bitValue = eventData >> bitOffset;
    Logging::LogSpam(__FUNCTION__) << std::hex << 
        " " << byteOffset << " " << bitOffset 
        << " [" << +bitValue << "]\n" << std::dec;
    return bitValue;
}

unsigned ReadEvent(FileBuffer& fb, unsigned eventPtr) 
{
    if (eventPtr >= 0xdac0)
    {
        const auto [byteOffset, bitOffset] = CalculateComplexEventOffset(eventPtr);
        return ReadBitValueAt(fb, byteOffset, bitOffset);
    }
    else
    {
        const auto [byteOffset, bitOffset] = CalculateEventOffset(eventPtr);
        return ReadBitValueAt(fb, byteOffset, bitOffset);
    }
}

bool ReadEventBool(FileBuffer& fb, unsigned eventPtr) 
{
    return (ReadEvent(fb, eventPtr) & 0x1) == 1;
}

}
