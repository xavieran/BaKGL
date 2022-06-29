#pragma once

#include "xbak/FileBuffer.h"

#include <cstdint>
#include <map>
#include <memory>

namespace BAK {

enum class SoundFormat
{
    Wave,
    Midi,
    Unknown

};

struct MidiEvent
{
    unsigned delta;
    unsigned size;
    std::uint8_t data[8];
};

class Sound
{
public:
    Sound(unsigned t);

    unsigned GetType() const;
    unsigned GetChannel() const;
    SoundFormat GetFormat() const;

    FileBuffer* GetSamples();
    void AddVoice(FileBuffer&);
    void GenerateBuffer();

private:
    unsigned int mType;
    unsigned int mChannel;
    SoundFormat mFormat;
    std::unique_ptr<FileBuffer> mBuffer;

    std::multimap<unsigned, MidiEvent> mMidiEvents;
    void PutVariableLength(FileBuffer& buf, unsigned n);
    void CreateWaveSamples(FileBuffer& buf);
    void CreateMidiEvents(FileBuffer& buf);
    void GenerateMidi();
    void GenerateWave();
};

}
