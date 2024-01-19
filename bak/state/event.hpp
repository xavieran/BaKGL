#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

void SetBitValueAt(FileBuffer&, unsigned byteOffset, unsigned bitOffset, unsigned value);

std::pair<unsigned, unsigned> CalculateComplexEventOffset(unsigned eventPtr);
std::pair<unsigned, unsigned> CalculateEventOffset(unsigned eventPtr);

void SetEventFlag(FileBuffer&, unsigned eventPtr, unsigned value);
void SetEventFlagTrue (FileBuffer&, unsigned eventPtr);
void SetEventFlagFalse(FileBuffer&, unsigned eventPtr);

unsigned ReadBitValueAt(FileBuffer&, unsigned byteOffset, unsigned bitOffset);
unsigned ReadEvent(FileBuffer&, unsigned eventPtr);
bool ReadEventBool(FileBuffer&, unsigned eventPtr);

}
