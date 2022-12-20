#pragma once

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "bak/fileBuffer.hpp"

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
            auto name = fb.GetString();
            Logging::LogDebug(__FUNCTION__) << "Spell: " << i << " " << name <<"\n";
            mSpellNames.emplace_back(std::move(name));
        }
    }

    std::string_view GetSpellName(unsigned spellIndex) const
    {
        ASSERT(spellIndex < mSpellNames.size());
        Logging::LogDebug(__FUNCTION__) << "Spell: " << spellIndex << " " << mSpellNames[spellIndex] <<"\n";
        return mSpellNames[spellIndex];
    }

private:
    std::vector<std::string> mSpellNames;
};

}
