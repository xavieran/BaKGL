#pragma once

#include "bak/encounter/eventFlag.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

template <bool isEnable>
EventFlagFactory<isEnable>::EventFlagFactory()
:
    mEventFlags{}
{
    Load();
}

template <bool isEnable>
EventFlag EventFlagFactory<isEnable>::Get(unsigned i) const
{
    assert(i < mEventFlags.size());
    return mEventFlags[i];
}

template <bool isEnable>
void EventFlagFactory<isEnable>::EventFlagFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        isEnable ? sEnable : sDisable);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(4);
        const auto eventPointer = fb.GetUint16LE();
        assert(fb.GetUint16LE() == 0);
        mEventFlags.emplace_back(eventPointer, isEnable);
    }
}

}
