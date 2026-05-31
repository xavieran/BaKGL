#pragma once

namespace BAK {
class FileBuffer;
class Conditions;
}

namespace BAK::State {

void SyncConditionEventFlags(
    FileBuffer& fb,
    unsigned charIndex,
    bool inCombat,
    const Conditions& conditions);

bool IsConditionEvent(unsigned eventPtr);
}
