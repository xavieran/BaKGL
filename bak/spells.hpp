#pragma once

#include "com/assert.hpp"

#include "xbak/FileBuffer.h"

#include <string_view>
#include <vector>

namespace BAK {

class SpellInfo
{
    static constexpr auto sSpellNamesFile = "SPELLS.DAT";
    static constexpr auto sSpellDocsFile  = "SPELLDOC.DAT";
public:
    SpellInfo()
    :
        mSpellNames{}
    {
        auto fb = FileBufferFactory::Get().CreateDataBuffer(sSpellNamesFile);
        const auto spells = fb.GetUint32LE();
        for (unsigned i = 0; i < spells; i++)
        {
            fb.DumpAndSkip(22);
        }

        for (unsigned i = 0; i < spells; i++)
        {
            mSpellNames.emplace_back(fb.GetString());
        }
    }

    std::string_view GetSpellName(unsigned spellIndex) const
    {
        ASSERT(spellIndex < mSpellNames.size());
        return mSpellNames[spellIndex];
    }

private:
    std::vector<std::string> mSpellNames;
};

}
