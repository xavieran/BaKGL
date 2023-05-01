#pragma once

#include "bak/encounter/eventFlag.hpp"

#include "com/assert.hpp"

#include "bak/fileBufferFactory.hpp"

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
    ASSERT(i < mEventFlags.size());
    return mEventFlags[i];
}

template <bool isEnable>
void EventFlagFactory<isEnable>::EventFlagFactory::Load()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        isEnable ? sEnable : sDisable);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto chance = fb.GetUint8();
        const auto eventPointer = fb.GetUint16LE();
        ASSERT(fb.GetUint16LE() == 0);
        mEventFlags.emplace_back(chance, eventPointer, isEnable);
    }
}

}
