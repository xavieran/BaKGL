#include "bak/monster.hpp"

#include "com/logger.hpp"

#include "com/assert.hpp"
#include "com/ostream.hpp"

#include "bak/fileBufferFactory.hpp"

#include <iostream>

namespace BAK {

const MonsterNames& MonsterNames::Get()
{
    static auto mnames = MonsterNames{};
    return mnames;
}

MonsterNames::Monster::Monster(
    std::string prefix,
    std::uint8_t unknown0,
    std::uint8_t unknown1,
    std::uint8_t unknown2,
    std::uint8_t colorSwap)
:
    mPrefix{prefix},
    mSuffix0{unknown0},
    mSuffix1{unknown1},
    mSuffix2{unknown2},
    mColorSwap{colorSwap}
{}

std::ostream& operator<<(std::ostream& os, const MonsterNames::Monster& m)
{
    os << "Monster{ Pref: " << m.mPrefix << " spriteSheets: [" << +m.mSuffix0
        << ", " << +m.mSuffix1 << ", " << +m.mSuffix2 
        << "] cs: " << +m.mColorSwap << "}";
    return os;
}

const std::string& MonsterNames::GetMonsterName(MonsterIndex monster) const
{
    if (monster.mValue < mMonsterNames.size())
    {
        return mMonsterNames[monster.mValue];
    }
    else
    {
        return sInvalidMonster;
    }
}

const std::string& MonsterNames::GetMonsterAnimationFile(MonsterIndex monster) const
{
    ASSERT(monster.mValue < mMonsterPrefixes.size());
    return mMonsterPrefixes[monster.mValue].mPrefix;
}

std::uint8_t MonsterNames::GetColorSwap(MonsterIndex monster) const
{
    ASSERT(monster.mValue < mMonsterPrefixes.size());
    return mMonsterPrefixes[monster.mValue].mColorSwap;
}

std::size_t MonsterNames::size() const
{
    return mMonsterPrefixes.size();
}

MonsterNames::MonsterNames()
:
    mMonsterNames{},
    mMonsterPrefixes{}
{
    const auto& logger = Logging::LogState::GetLogger("MonsterNames");
    {
        auto fb = FileBufferFactory::Get().CreateDataBuffer("MNAMES.DAT");
        const auto monsters = fb.GetUint32LE();
        logger.Debug() << "Loading monsters: " << "\n";
        logger.Debug() << "Monsters: " << monsters << "\n";

        std::vector<unsigned> offsets{};
        for (unsigned i = 0; i < monsters; i++)
        {
            const auto& offset = offsets.emplace_back(fb.GetUint16LE());
            logger.Debug() << "I: " << i << " " 
                << std::hex << offset << std::dec << std::endl;
        }

        auto start = fb.Tell();
        unsigned p = 0;
        mMonsterNames.emplace_back(sInvalidMonster);
        for (auto offset : offsets)
        {
            if (start + offset > fb.GetSize())
            {
                logger.Debug() << "Seeking past end of file! offset is: " 
                    << offset << " start is: " << start << " fsize: " << fb.GetSize() << " \n";
                mMonsterNames.emplace_back(sInvalidMonster);
                continue;
            }
            fb.Seek(start + offset);
            const auto string = fb.GetString();
            logger.Debug() << p++ << " " << string << std::endl;
            mMonsterNames.emplace_back(string);
        }

        for (unsigned i = 0; i < mMonsterPrefixes.size(); i ++)
        {
            logger.Debug() << "K: " << i << " : " << mMonsterNames[i] << "\n";
        }
    }
    {
        auto fb = FileBufferFactory::Get().CreateDataBuffer("BNAMES.DAT");
        auto monsters = fb.GetUint32LE();
        logger.Debug() << "Loading keywords" << "\n";
        logger.Debug() << "Length: " << monsters << "\n";

        std::vector<unsigned> offsets{};
        for (unsigned i = 0; i < monsters; i++)
        {
            const auto& offset = offsets.emplace_back(fb.GetUint16LE());
            logger.Debug() << "I: " << i << " " 
                << std::hex << offset << std::dec << std::endl;
        }

        auto start = fb.Tell();
        unsigned p = 0;
        mMonsterPrefixes.emplace_back("", 0, 0, 0, 255);
        for (auto offset : offsets)
        {
            if (start + offset > fb.GetSize()) continue;
            fb.Seek(start + offset);
            const auto prefix = fb.GetString();
            const auto unk0 = fb.GetUint8();
            const auto unk1 = fb.GetUint8();
            const auto unk2 = fb.GetUint8();
            const auto unk3 = fb.GetUint8();
            logger.Debug() << p++ << " " << prefix << " " 
                << +unk0 << " " 
                << +unk1 << " " 
                << +unk2 << " " 
                << +unk3 << " " << std::endl;
            mMonsterPrefixes.emplace_back(prefix, unk0, unk1, unk2, unk3);
        }
    }
}

}
