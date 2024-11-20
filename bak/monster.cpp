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
    mUnknown0{unknown0},
    mUnknown1{unknown1},
    mUnknown2{unknown2},
    mColorSwap{colorSwap}
{}

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
        logger.Spam() << "Loading monsters: " << "\n";
        logger.Spam() << "Monsters: " << monsters << "\n";

        std::vector<unsigned> offsets{};
        for (unsigned i = 0; i < monsters; i++)
        {
            const auto& offset = offsets.emplace_back(fb.GetUint16LE());
            logger.Spam() << "I: " << i << " " 
                << std::hex << offset << std::dec << std::endl;
        }

        std::vector<std::string> strings{};
        auto start = fb.Tell();
        unsigned p = 0;
        for (auto offset : offsets)
        {
            if (start + offset > fb.GetSize())
            {
                logger.Spam() << "Seeking past end of file!\n";
                strings.emplace_back("INVALID MONSTER");
                mMonsterNames.emplace_back("INVALID MONSTER");
                continue;
            }
            fb.Seek(start + offset);
            const auto& keyword = strings.emplace_back(fb.GetString());
            logger.Spam() << p++ << " " << keyword << std::endl;
            mMonsterNames.emplace_back(keyword);
        }

        for (unsigned i = 0; i < strings.size(); i ++)
        {
            logger.Spam() << "K: " << i << " : " << strings[i] << "\n";
        }
    }
    {
        auto fb = FileBufferFactory::Get().CreateDataBuffer("BNAMES.DAT");
        auto monsters = fb.GetUint32LE();
        logger.Spam() << "Loading keywords" << "\n";
        logger.Spam() << "Length: " << monsters << "\n";

        std::vector<unsigned> offsets{};
        for (unsigned i = 0; i < monsters; i++)
        {
            const auto& offset = offsets.emplace_back(fb.GetUint16LE());
            logger.Spam() << "I: " << i << " " 
                << std::hex << offset << std::dec << std::endl;
        }

        std::vector<std::string> strings{};
        auto start = fb.Tell();
        unsigned p = 0;
        for (auto offset : offsets)
        {
            if (start + offset > fb.GetSize()) continue;
            fb.Seek(start + offset);
            const auto prefix = fb.GetString();
            const auto unk0 = fb.GetUint8();
            const auto unk1 = fb.GetUint8();
            const auto unk2 = fb.GetUint8();
            const auto unk3 = fb.GetUint8();
            logger.Spam() << p++ << " " << prefix << " " 
                << +unk0 << " " 
                << +unk1 << " " 
                << +unk2 << " " 
                << +unk3 << " " << std::endl;
            mMonsterPrefixes.emplace_back(prefix, unk0, unk1, unk2, unk3);
        }
    }
}

}
