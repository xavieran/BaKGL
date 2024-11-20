#pragma once

#include "bak/sound.hpp"

#include <unordered_map>
#include <vector>

namespace BAK {

class SoundData
{
public:
    SoundData(
        std::string name,
        unsigned type,
        std::vector<Sound>&& sounds);

    std::vector<Sound>& GetSounds();

private:
    std::string mName;
    unsigned mType;
    std::vector<Sound> mSounds;
};

class SoundStore
{
    static constexpr auto sSoundFile = "frp.sx";

public:
    static SoundStore& Get();

    SoundData& GetSoundData(unsigned id);

private:
    SoundStore();

    std::unordered_map<unsigned, SoundData> mSoundMap;
};

}
