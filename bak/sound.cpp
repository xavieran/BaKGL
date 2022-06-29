#include "bak/sound.hpp"

#include <cstring>
#include <iostream>

namespace BAK {

Sound::Sound(unsigned t)
:
    mType(t),
    mChannel(255),
    mFormat(SoundFormat::Unknown),
    mBuffer(),
    mMidiEvents()
{
}

unsigned int Sound::GetType() const
{
    return mType;
}

unsigned int Sound::GetChannel() const
{
    return mChannel;
}

SoundFormat Sound::GetFormat() const
{
    return mFormat;
}

FileBuffer* Sound::GetSamples()
{
    return mBuffer.get();
}

/* WAVE/RIFF tags & constants */
const uint32_t RIFF_ID         = 0x46464952;
const uint32_t WAVE_ID         = 0x45564157;
const uint32_t FMT_ID          = 0x20746d66;
const uint32_t DATA_ID         = 0x61746164;

void Sound::CreateWaveSamples(FileBuffer& buf)
{
    buf.Skip(1);
    unsigned int rate = buf.GetUint16LE();
    unsigned int size = buf.GetUint32LE();
    buf.Skip(2);
    mBuffer = std::make_unique<FileBuffer>(12 + 8 + 16 + 8 + size);
    mBuffer->PutUint32LE(RIFF_ID);
    mBuffer->PutUint32LE(mBuffer->GetSize() - 8);
    mBuffer->PutUint32LE(WAVE_ID);
    mBuffer->PutUint32LE(FMT_ID);
    mBuffer->PutUint32LE(16);      // chunk size
    mBuffer->PutUint16LE(1);       // compression: 1 = uncompressed PCM
    mBuffer->PutUint16LE(1);       // # mChannels
    mBuffer->PutUint32LE(rate);    // sample rate
    mBuffer->PutUint32LE(rate);    // average bytes per sec: sample rate * block align
    mBuffer->PutUint16LE(1);       // block align: significant bits per sample / 8 * # mChannels
    mBuffer->PutUint16LE(8);       // significant bits per sample
    mBuffer->PutUint32LE(DATA_ID);
    mBuffer->PutUint32LE(size);
    mBuffer->CopyFrom(&buf, size);
    mBuffer->Rewind();
}

void Sound::GenerateWave()
{
}

/* Standard MIDI File tags & constants */
const uint32_t SMF_HEADER      = 0x6468544d;
const uint32_t SMF_TRACK       = 0x6b72544d;
const uint16_t SMF_FORMAT      = 0;
const uint32_t SMF_HEADER_SIZE = 6;
const uint16_t SMF_PPQN        = 32;

/* MIDI event codes */
const uint8_t MIDI_NOTE_OFF  = 0x80;
const uint8_t MIDI_NOTE_ON   = 0x90;
const uint8_t MIDI_KEY       = 0xa0;
const uint8_t MIDI_CONTROL   = 0xb0;
const uint8_t MIDI_PATCH     = 0xc0;
const uint8_t MIDI_CHANNEL   = 0xd0;
const uint8_t MIDI_PITCH     = 0xe0;
const uint8_t MIDI_SYSEX     = 0xf0;
const uint8_t MIDI_TIMING    = 0xf8;
const uint8_t MIDI_SEQ_START = 0xfa;
const uint8_t MIDI_SEQ_CONT  = 0xfb;
const uint8_t MIDI_SEQ_END   = 0xfc;
const uint8_t MIDI_META      = 0xff;

/* MIDI Meta events */
const uint8_t META_SEQNUM     = 0x00;
const uint8_t META_TEXT       = 0x01;
const uint8_t META_COPYRIGHT  = 0x02;
const uint8_t META_TRACK      = 0x03;
const uint8_t META_INSTRUMENT = 0x04;
const uint8_t META_LYRIC      = 0x05;
const uint8_t META_MARKER     = 0x06;
const uint8_t META_CUE        = 0x07;
const uint8_t META_CHANNEL    = 0x20;
const uint8_t META_PORT       = 0x21;
const uint8_t META_EOT        = 0x2f;
const uint8_t META_TEMPO      = 0x51;
const uint8_t META_SMPTE      = 0x54;
const uint8_t META_TIME       = 0x58;
const uint8_t META_KEY        = 0x59;
const uint8_t META_SEQDATA    = 0x7f;

void Sound::PutVariableLength(FileBuffer& buf, unsigned n)
{
    unsigned int tmp = (n & 0x7f);
    unsigned int k = 1;
    while (n >>= 7)
    {
        tmp <<= 8;
        tmp |= ((n & 0x7f) | 0x80);
        k++;
    }
    while (k--)
    {
        buf.PutUint8(tmp & 0xff);
        tmp >>= 8;
    }
}

void Sound::CreateMidiEvents(FileBuffer& buf)
{
    unsigned delta{};
    unsigned code{};
    unsigned mode{};
    unsigned tick{};

    buf.Skip(1);
    while ((mode != MIDI_SEQ_END) && !buf.AtEnd())
    {
        delta = 0;
        code = buf.GetUint8();
        while (code == MIDI_TIMING)
        {
            delta += 240;
            code = buf.GetUint8();
        }
        delta += code;
        code = buf.GetUint8();
        if (   ((code & 0xf0) == MIDI_NOTE_ON)
            || ((code & 0xf0) == MIDI_CONTROL)
            || ((code & 0xf0) == MIDI_PATCH)
            || ((code & 0xf0) == MIDI_PITCH))
        {
            mode = code;
            if ((code & 0x0f) != mChannel)
            {
                throw std::runtime_error("Data corruption in sound data");
            }
        }
        else if (code == MIDI_SEQ_END)
        {
            mode = code;
        }
        else
        {
            buf.Skip(-1);
        }
        if (mode != MIDI_SEQ_END)
        {
            MidiEvent me;
            memset(&me, 0, sizeof(MidiEvent));
            me.data[0] = mode;
            switch (mode & 0xf0)
            {
            case MIDI_NOTE_ON:
                me.data[1] = buf.GetUint8();
                me.data[2] = buf.GetUint8();
                if (me.data[2] == 0)
                {
                    me.data[0] = MIDI_NOTE_OFF | mChannel;
                }
                me.size = 3;
                break;
            case MIDI_CONTROL:
            case MIDI_PITCH:
                me.data[1] = buf.GetUint8();
                me.data[2] = buf.GetUint8();
                me.size = 3;
                break;
            case MIDI_PATCH:
                me.data[1] = buf.GetUint8();
                me.size = 2;
                break;
            default:
                if (mode == MIDI_SEQ_END)
                {
                    me.size = 1;
                }
                else
                {
                    throw std::runtime_error("Data corruption in sound file");
                }
                break;
            }

            tick += delta;
            mMidiEvents.emplace(tick, me);
        }
    }
}

void Sound::GenerateMidi()
{
    unsigned int size = 0;
    unsigned int previousTick = 0;

    for (auto& [tick, me] : mMidiEvents)
    {
        me.delta = tick - previousTick;
        size += 1;
        if (me.delta >= (1 << 7))
        {
            size += 1;
        }
        if (me.delta >= (1 << 14))
        {
            size += 1;
        }
        if (me.delta >= (1 << 21))
        {
            size += 1;
        }
        size += me.size;
        previousTick = tick;
    }

    mBuffer = std::make_unique<FileBuffer>(8 + SMF_HEADER_SIZE + 8 + size + 4);
    mBuffer->PutUint32LE(SMF_HEADER);
    mBuffer->PutUint32BE(SMF_HEADER_SIZE);
    mBuffer->PutUint16BE(SMF_FORMAT);
    mBuffer->PutUint16BE(1);
    mBuffer->PutUint16BE(SMF_PPQN);
    mBuffer->PutUint32LE(SMF_TRACK);
    mBuffer->PutUint32BE(size);

    for (const auto& [_, me] : mMidiEvents)
    {
        PutVariableLength(*mBuffer, me.delta);
        for (unsigned i = 0; i < me.size; i++)
        {
            mBuffer->PutUint8(me.data[i]);
        }
    }

    mMidiEvents.clear();
    mBuffer->PutUint8(0);
    mBuffer->PutUint8(MIDI_META);
    mBuffer->PutUint8(META_EOT);
    mBuffer->PutUint8(0);
    mBuffer->Rewind();
}

void Sound::AddVoice(FileBuffer& buf)
{
    unsigned int code = buf.GetUint8();
    mChannel = code & 0x0f;
    if (code == 0xfe)
    {
        mFormat = SoundFormat::Wave;
        CreateWaveSamples(buf);
    }
    else
    {
        mFormat = SoundFormat::Midi;
        CreateMidiEvents(buf);
    }
}

void Sound::GenerateBuffer()
{
    if (mFormat == SoundFormat::Midi)
    {
        GenerateMidi();
    }
}

}
