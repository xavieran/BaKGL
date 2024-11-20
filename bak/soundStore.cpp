#include "bak/soundStore.hpp"

#include "bak/dataTags.hpp"
#include "bak/tags.hpp"

namespace BAK {

SoundData::SoundData(
    std::string name,
    unsigned type,
    std::vector<Sound>&& sounds)
:
    mName{name},
    mType{type},
    mSounds{std::move(sounds)}
{
}

std::vector<Sound>& SoundData::GetSounds() { return mSounds; }


SoundStore& SoundStore::Get()
{
    static SoundStore soundStore{};
    return soundStore;
}

SoundData& SoundStore::GetSoundData(unsigned id)
{
    return mSoundMap.at(id);
}

SoundStore::SoundStore()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(sSoundFile);

    auto infbuf = fb.Find(DataTag::INF);
    auto tagbuf = fb.Find(DataTag::TAG);

    infbuf.Skip(2);
    unsigned n = infbuf.GetUint16LE();
    infbuf.Skip(1);
    Tags tags{};
    tags.Load(tagbuf);

    for (unsigned i = 0; i < n; i++)
    {
        unsigned id = infbuf.GetUint16LE();
        std::streamoff offset = infbuf.GetUint32LE();
        if (const auto name = tags.GetTag(Tag{id}))
        {
            fb.Seek(offset + 8);
            if (id != fb.GetUint16LE())
            {
                throw std::runtime_error("Data corruption in sound file");
            }
            unsigned soundType = fb.GetUint8();

            fb.Skip(2);
            const auto size = fb.GetUint32LE();
            auto sndbuf = FileBuffer{size - 2};
            fb.Skip(2);
            sndbuf.Fill(&fb);
            fb.Skip(-sndbuf.GetSize());

            std::vector<Sound> sounds{};

            int code = fb.GetUint8();
            while (code != 0xff)
            {
                Sound& sound = sounds.emplace_back(code);

                std::vector<unsigned int> offsetVec;
                std::vector<unsigned int> sizeVec;
                code = fb.GetUint8();
                while (code != 0xff)
                {
                    fb.Skip(1);
                    offsetVec.push_back(fb.GetUint16LE());
                    sizeVec.push_back(fb.GetUint16LE());
                    code = fb.GetUint8();
                }
                for (unsigned int j = 0; j < offsetVec.size(); j++)
                {
                    sndbuf.Seek(offsetVec[j]);
                    auto samplebuf = FileBuffer{sizeVec[j]};
                    samplebuf.Fill(&sndbuf);
                    sound.AddVoice(samplebuf);
                }
                sound.GenerateBuffer();
                code = fb.GetUint8();
            }

            mSoundMap.try_emplace(id, *name, soundType, std::move(sounds));
        }
        else
        {
            throw std::runtime_error("Data corruption in sound file");
        }
    }
}

}
