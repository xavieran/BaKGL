#include "bak/fileBuffer.hpp"

#include "com/logger.hpp"
#include "com/path.hpp"

#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>

#include <cassert>
#include <cstring>
#include <limits>
#include <sstream>

#include <SDL2/SDL_endian.h>

namespace BAK {

FileBuffer::FileBuffer(
    std::uint8_t* buf,
    std::uint8_t* current,
    std::uint32_t size,
    std::uint32_t nb)
:
    mBuffer{buf},
    mCurrent{current},
    mSize{size},
    mNextBit{nb},
    mOwnBuffer{false}
{
}

FileBuffer::FileBuffer(const unsigned n)
:
    mBuffer{std::invoke([n](){
        auto buf = new uint8_t[n];
        memset(buf, 0, n);
        return buf;
    })},
    mCurrent{mBuffer},
    mSize{n},
    mNextBit{0},
    mOwnBuffer{true}
{
}

FileBuffer::FileBuffer(FileBuffer&& fb) noexcept
{
    (*this) = std::move(fb);
}

FileBuffer& FileBuffer::operator=(FileBuffer&& fb) noexcept
{
    fb.mOwnBuffer = false;

    mBuffer = fb.mBuffer;
    mCurrent = fb.mCurrent;
    mSize = fb.mSize;
    mNextBit = fb.mNextBit;
    mOwnBuffer = true;
    return *this;
}

FileBuffer::~FileBuffer()
{
    if (mBuffer && mOwnBuffer)
    {
        delete[] mBuffer;
    }
}

void
FileBuffer::CopyFrom(FileBuffer *buf, const unsigned n)
{
    if (mBuffer && n && (mCurrent + n <= mBuffer + mSize))
    {
        buf->GetData(mCurrent, n);
        mCurrent += n;
    }
}

void
FileBuffer::CopyTo(FileBuffer *buf, const unsigned n)
{
    if (mBuffer && n && (mCurrent + n <= mBuffer + mSize))
    {
        buf->PutData(mCurrent, n);
        mCurrent += n;
    }
}

void
FileBuffer::Fill(FileBuffer *buf)
{
    if (mBuffer)
    {
        mCurrent = mBuffer;
        buf->GetData(mBuffer, std::min(mSize, buf->GetSize()));
    }
}

FileBuffer FileBuffer::Find(std::uint32_t tag) const
{
    auto *search = mBuffer;
    for (; search < (mBuffer + mSize - sizeof(std::uint32_t)); search++)
    {
        auto current = *reinterpret_cast<std::uint32_t*>(search);
        if (current == tag)
        {
            search += 4;
            const auto mBufferSize = *reinterpret_cast<std::uint32_t*>(search);
            search += 4;
            return FileBuffer{
                search,
                search,
                mBufferSize,
                0};
        }
    }

    std::stringstream ss{};
    ss << "Tag not found: " << std::hex << tag;
    throw std::runtime_error(ss.str());
}

FileBuffer FileBuffer::MakeSubBuffer(std::uint32_t offset, std::uint32_t size) const
{
    if (mSize < offset + size)
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " Requested new FileBuffer larger than available size: ("
            << offset << ", " << size << ") my size: " << mSize;
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }

    return FileBuffer{
        mBuffer + offset,
        mBuffer + offset,
        size,
        0};
}

void
FileBuffer::Load(std::ifstream &ifs)
{
    if (ifs.is_open())
    {
        mCurrent = mBuffer;
        ifs.read((char *)mBuffer, mSize);
        if (ifs.fail())
        {
            std::stringstream ss{};
            ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " IOError!";
            Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " OpenError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void
FileBuffer::Save(std::ofstream &ofs)
{
    if (ofs.is_open())
    {
        mCurrent = mBuffer;
        ofs.write((char *)mBuffer, mSize);
        if (ofs.fail())
        {
            std::stringstream ss{};
            ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " IOError!";
            Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " OpenError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void
FileBuffer::Save(std::ofstream &ofs, const unsigned n)
{
    if (ofs.is_open())
    {
        if (n <= mSize)
        {
            mCurrent = mBuffer;
            ofs.write((char *)mBuffer, n);
            if (ofs.fail())
            {
                std::stringstream ss{};
                ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " IOError!";
                Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        else
        {
            std::stringstream ss{};
            ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferEmpty!";
            Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " OpenError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}


void
FileBuffer::Dump(const unsigned n)
{
    if (n == 0) return;
    Dump(std::cout, n);
}

void
FileBuffer::DumpAndSkip(const unsigned n)
{
    if (n == 0) return;
    Dump(n);
    Skip(n);
}

void
FileBuffer::Dump(std::ostream& os, const unsigned n)
{
    uint8_t* tmp = mCurrent;
    unsigned count = 0;
    os << std::setbase(16) << std::setfill('0') << std::setw(8) << count << ": ";
    while ((tmp < (mBuffer + mSize)) && ((tmp < (mCurrent + n)) || (n == 0)))
    {
        os << std::setw(2) << (unsigned)*tmp++ << " ";
        if ((++count & 0x1f) == 0)
        {
            os << std::endl << std::setw(8) << count << ": ";
        }
        else if ((count & 0x07) == 0)
        {
            os << "| ";
        }
    }
    os << std::setbase(10) << std::setfill(' ') << std::endl;
}

void
FileBuffer::Seek(const unsigned n)
{
    if ((mCurrent) && (n <= mSize))
    {
        mCurrent = mBuffer + n;
    }
}

void
FileBuffer::Skip(const int n)
{
    if ((mCurrent) && (mCurrent + n <= mBuffer + mSize))
    {
        mCurrent += n;
    }
}

void
FileBuffer::SkipBits()
{
    if (mNextBit)
    {
        Skip(1);
        mNextBit = 0;
    }
}

typedef union _HashTableEntry {
    uint32_t code;
    struct
    {
        uint16_t prefix;
        uint8_t  append;
    }
    entry;
} HashTableEntry;

unsigned
FileBuffer::CompressLZW(FileBuffer *result)
{
    try
    {
        std::map<uint32_t, uint16_t> hashtable;
        unsigned n_bits = 9;
        unsigned free_entry = 257;
        unsigned bitpos = 0;
        HashTableEntry hte;
        hte.entry.prefix = GetUint8();
        while (!AtEnd() && !result->AtEnd())
        {
            hte.entry.append = GetUint8();
            std::map<uint32_t, uint16_t>::iterator it = hashtable.find(hte.code);
            if (it == hashtable.end())
            {
                result->PutBits(hte.entry.prefix, n_bits);
                bitpos += n_bits;
                hashtable.insert(std::pair<uint32_t, uint16_t>(hte.code, free_entry));
                hte.entry.prefix = hte.entry.append;
                free_entry++;
                if (free_entry >= (unsigned)(1 << n_bits))
                {
                    if (n_bits < 12)
                    {
                        n_bits++;
                    }
                    else
                    {
                        hashtable.clear();
                        free_entry = 256;
                        result->PutBits(free_entry, n_bits);
                        result->SkipBits();
                        result->Skip((((bitpos-1)+((n_bits<<3)-(bitpos-1+(n_bits<<3))%(n_bits<<3)))-bitpos)>>3);
                        n_bits = 9;
                        bitpos = 0;
                    }
                }
            }
            else
            {
                hte.entry.prefix = it->second;
            }
        }
        hashtable.clear();
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::CompressLZSS(FileBuffer *result)
{
    try
    {
        uint8_t *data = GetCurrent();
        uint8_t *current = GetCurrent();
        uint8_t *codeptr = result->GetCurrent();
        uint8_t byte = GetUint8();
        uint8_t code = 0;
        uint8_t mask = 0;
        while (!AtEnd() && !result->AtEnd())
        {
            if (!mask)
            {
                *codeptr = code;
                codeptr = result->GetCurrent();
                result->Skip(1);
                code = 0;
                mask = 0x01;
            }
            unsigned off = 0;
            unsigned len = 0;
            uint8_t *ptr = current;
            while (ptr > data)
            {
                ptr--;
                if (*ptr == byte)
                {
                    off = ptr - data;
                    len = 1;
                    while ((current + len < mBuffer + mSize) && (ptr[len] == current[len]))
                    {
                        len++;
                    }
                }
            }
            if (len < 5)
            {
                code |= mask;
                result->PutUint8(byte);
            }
            else
            {
                result->PutUint16LE(off);
                result->PutUint8(len - 5);
                Skip(len - 1);
            }
            current = GetCurrent();
            byte = GetUint8();
            mask <<= 1;
        }
        *codeptr = code;
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::CompressRLE(FileBuffer *result)
{
    try
    {
        uint8_t *skipptr = GetCurrent();
        uint8_t byte = 0;
        uint8_t next = GetUint8();
        unsigned count;
        unsigned skipped = 0;
        while (!AtEnd() && !result->AtEnd())
        {
            count = 1;
            do
            {
                byte = next;
                next = GetUint8();
                count++;
            }
            while (!AtEnd() && (next == byte));
            if (next != byte)
            {
                count--;
            }
            if (count > 3)
            {
                if (skipped > 0)
                {
                    while (skipped > 0)
                    {
                        unsigned n;
                        if (skipped > 127)
                        {
                            n = 127;
                        }
                        else
                        {
                            n = skipped & 0x7f;
                        }
                        result->PutUint8(n);
                        result->PutData(skipptr, n);
                        skipped -= n;
                        skipptr += n;
                    }
                }
                while (count > 3)
                {
                    unsigned n;
                    if (count > 127)
                    {
                        n = 127;
                    }
                    else
                    {
                        n = count & 0x7f;
                    }
                    result->PutUint8(n | 0x80);
                    result->PutUint8(byte);
                    count -= n;
                }
                skipped = count;
                skipptr = GetCurrent() - skipped - 1;
            }
            else
            {
                skipped += count;
            }
        }
        if (next != byte)
        {
            skipped++;
        }
        if (skipped > 0)
        {
            Skip(-skipped);
            while (skipped > 0)
            {
                unsigned n = skipped & 0x7f;
                result->PutUint8(n);
                result->CopyFrom(this, n);
                skipped -= n;
            }
        }
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::Compress(FileBuffer *result, const unsigned method)
{
    switch (method)
    {
    case COMPRESSION_LZW:
        return CompressLZW(result);
        break;
    case COMPRESSION_LZSS:
        return CompressLZSS(result);
        break;
    case COMPRESSION_RLE:
        return CompressRLE(result);
        break;
    default:
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " CompressionError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
        break;
    }
}

typedef struct _CodeTableEntry
{
    uint16_t prefix;
    uint8_t  append;
}
CodeTableEntry;

unsigned
FileBuffer::DecompressLZW(FileBuffer *result)
{
    try
    {
        CodeTableEntry *codetable = new CodeTableEntry[4096];
        uint8_t *decodestack = new uint8_t[4096];
        uint8_t *stackptr = decodestack;
        unsigned n_bits = 9;
        unsigned free_entry = 257;
        unsigned oldcode = GetBits(n_bits);
        unsigned lastbyte = oldcode;
        unsigned bitpos = 0;
        result->PutUint8(oldcode);
        while (!AtEnd() && !result->AtEnd())
        {
            unsigned newcode = GetBits(n_bits);
            bitpos += n_bits;
            if (newcode == 256)
            {
                SkipBits();
                Skip((((bitpos-1)+((n_bits<<3)-(bitpos-1+(n_bits<<3))%(n_bits<<3)))-bitpos)>>3);
                n_bits = 9;
                free_entry = 256;
                bitpos = 0;
            }
            else
            {
                unsigned code = newcode;
                if (code >= free_entry)
                {
                    *stackptr++ = lastbyte;
                    code = oldcode;
                }
                while (code >= 256)
                {
                    *stackptr++ = codetable[code].append;
                    code = codetable[code].prefix;
                }
                *stackptr++ = code;
                lastbyte = code;
                while (stackptr > decodestack)
                {
                    result->PutUint8(*--stackptr);
                }
                if (free_entry < 4096)
                {
                    codetable[free_entry].prefix = oldcode;
                    codetable[free_entry].append = lastbyte;
                    free_entry++;
                    if ((free_entry >= (unsigned)(1 << n_bits)) && (n_bits < 12))
                    {
                        n_bits++;
                        bitpos = 0;
                    }
                }
                oldcode = newcode;
            }
        }
        delete[] decodestack;
        delete[] codetable;
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::DecompressLZSS(FileBuffer *result)
{
    try
    {
        uint8_t *data = result->GetCurrent();
        uint8_t code = 0;
        uint8_t mask = 0;
        while (!AtEnd() && !result->AtEnd())
        {
            if (!mask)
            {
                code = GetUint8();
                mask = 0x01;
            }
            if (code & mask)
            {
                result->PutUint8(GetUint8());
            }
            else
            {
                unsigned off = GetUint16LE();
                unsigned len = GetUint8() + 5;
                result->PutData(data + off, len);
            }
            mask <<= 1;
        }
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::DecompressRLE(FileBuffer *result)
{
    try
    {
        while (!AtEnd() && !result->AtEnd())
        {
            uint8_t control = GetUint8();
            if (control & 0x80)
            {
                result->PutData(GetUint8(), control & 0x7f);
            }
            else
            {
                result->CopyFrom(this, control);
            }
        }
        unsigned res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (std::exception& e)
    {
        Logging::LogFatal("FileBuffer") << __FUNCTION__ << " " << 
            e.what() << std::endl;
        throw;
    }
    return 0;
}

unsigned
FileBuffer::Decompress(FileBuffer *result, const unsigned method)
{
    switch (method)
    {
    case COMPRESSION_LZW:
        if ((GetUint8() != 0x02) || (GetUint32LE() != result->GetSize()))
        {
            std::stringstream ss{};
            ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " DataCorruption!";
            Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
        return DecompressLZW(result); break;
    case COMPRESSION_LZSS:
        return DecompressLZSS(result);
        break;
    case COMPRESSION_RLE:
        return DecompressRLE(result);
        break;
    default:
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " CompressionError!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
        break;
    }
}

bool
FileBuffer::AtEnd() const
{
    return (mCurrent >= mBuffer + mSize);
}

unsigned
FileBuffer::GetSize() const
{
    return mSize;
}

unsigned
FileBuffer::GetBytesDone() const
{
    return (mCurrent - mBuffer);
}

unsigned
FileBuffer::GetBytesLeft() const
{
    return (mBuffer + mSize - mCurrent);
}

uint8_t *
FileBuffer::GetCurrent() const
{
    return mCurrent;
}

unsigned
FileBuffer::GetNextBit() const
{
    return mNextBit;
}

void
FileBuffer::Rewind()
{
    mCurrent = mBuffer;
}

unsigned
FileBuffer::Tell()
{
    return mCurrent - mBuffer;
}

uint8_t
FileBuffer::GetUint8()
{
    uint8_t n;
    GetData(&n, 1);
    return n;
}

uint16_t
FileBuffer::GetUint16LE()
{
    uint16_t n;
    GetData(&n, 2);
    return SDL_SwapLE16(n);
}

uint16_t
FileBuffer::GetUint16BE()
{
    uint16_t n;
    GetData(&n, 2);
    return SDL_SwapBE16(n);
}

uint32_t
FileBuffer::GetUint32LE()
{
    uint32_t n;
    GetData(&n, 4);
    return SDL_SwapLE32(n);
}

uint32_t
FileBuffer::GetUint32BE()
{
    uint32_t n;
    GetData(&n, 4);
    return SDL_SwapBE32(n);
}

int8_t
FileBuffer::GetSint8()
{
    int8_t n;
    GetData(&n, 1);
    return n;
}

int16_t
FileBuffer::GetSint16LE()
{
    int16_t n;
    GetData(&n, 2);
    return SDL_SwapLE16(n);
}

int16_t
FileBuffer::GetSint16BE()
{
    int16_t n;
    GetData(&n, 2);
    return SDL_SwapBE16(n);
}

int32_t
FileBuffer::GetSint32LE()
{
    int32_t n;
    GetData(&n, 4);
    return SDL_SwapLE32(n);
}

int32_t
FileBuffer::GetSint32BE()
{
    int32_t n;
    GetData(&n, 4);
    return SDL_SwapBE32(n);
}

std::string
FileBuffer::GetString()
{
    if (mCurrent)
    {
        std::string s((char *)mCurrent);
        if ((mCurrent + s.length() + 1) <= (mBuffer + mSize))
        {
            mCurrent += s.length() + 1;
            return s;
        }
        else
        {
            std::stringstream ss{};
            ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferEmpty!";
            Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
    return "";
}

std::string
FileBuffer::GetString(const unsigned len)
{
    if ((mCurrent) && (mCurrent + len <= mBuffer + mSize))
    {
        std::string s((char *)mCurrent);
        mCurrent += len;
        return s;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferEmpty!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
    return "";
}

void
FileBuffer::GetData(void *data,
                    const unsigned n)
{
    if ((mCurrent + n) <= (mBuffer + mSize))
    {
        memcpy(data, mCurrent, n);
        mCurrent += n;
    }
    else
    {
        std::cerr << "Requested: " << n << " but @" << std::hex 
            << (mCurrent - mBuffer) << " mSize: " << std::dec << mSize
            << " @: " << std::hex << (mCurrent + n) << " to "
            << " @: " << std::hex << (mBuffer + mSize)
            << std::endl;
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferEmpty!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

unsigned
FileBuffer::GetBits(const unsigned n)
{
    if (mCurrent + ((mNextBit + n + 7)/8) <= mBuffer + mSize)
    {
        unsigned x = 0;
        for (unsigned i = 0; i < n; i++)
        {
            if (*mCurrent & (1 << mNextBit))
            {
                x += (1 << i);
            }
            mNextBit++;
            if (mNextBit > 7)
            {
                mCurrent++;
                mNextBit = 0;
            }
        }
        return x;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferEmpty!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void
FileBuffer::PutUint8(const uint8_t x)
{
    uint8_t xx = x;
    PutData(&xx, 1);
}

void
FileBuffer::PutUint16LE(const uint16_t x)
{
    uint16_t xx = SDL_SwapLE16(x);
    PutData(&xx, 2);
}

void
FileBuffer::PutUint16BE(const uint16_t x)
{
    uint16_t xx = SDL_SwapBE16(x);
    PutData(&xx, 2);
}

void
FileBuffer::PutUint32LE(const uint32_t x)
{
    uint32_t xx = SDL_SwapLE32(x);
    PutData(&xx, 4);
}

void
FileBuffer::PutUint32BE(const uint32_t x)
{
    uint32_t xx = SDL_SwapBE32(x);
    PutData(&xx, 4);
}

void
FileBuffer::PutSint8(const int8_t x)
{
    int8_t xx = x;
    PutData(&xx, 1);
}

void
FileBuffer::PutSint16LE(const int16_t x)
{
    int16_t xx = SDL_SwapLE16(x);
    PutData(&xx, 2);
}

void
FileBuffer::PutSint16BE(const int16_t x)
{
    int16_t xx = SDL_SwapBE16(x);
    PutData(&xx, 2);
}

void
FileBuffer::PutSint32LE(const int32_t x)
{
    int32_t xx = SDL_SwapLE32(x);
    PutData(&xx, 4);
}

void
FileBuffer::PutSint32BE(const int32_t x)
{
    int32_t xx = SDL_SwapBE32(x);
    PutData(&xx, 4);
}

void
FileBuffer::PutString(const std::string s)
{
    if ((mCurrent) && (mCurrent + s.length() + 1 <= mBuffer + mSize))
    {
        strncpy((char *)mCurrent, s.c_str(), s.length() + 1);
        mCurrent += s.length() + 1;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferFull!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void
FileBuffer::PutString(const std::string s, const unsigned len)
{
    if ((mCurrent) && (mCurrent + len <= mBuffer + mSize))
    {
        memset(mCurrent, 0, len);
        strncpy((char *)mCurrent, s.c_str(), len);
        mCurrent += len;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferFull!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void
FileBuffer::PutData(void *data, const unsigned n)
{
    if (mCurrent + n <= mBuffer + mSize)
    {
        memcpy(mCurrent, data, n);
        mCurrent += n;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferFull!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void FileBuffer::PutData(const uint8_t x, const unsigned n)
{
    if (mCurrent + n <= mBuffer + mSize)
    {
        memset(mCurrent, x, n);
        mCurrent += n;
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferFull!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void FileBuffer::PutBits(const unsigned x, const unsigned n)
{
    if (mCurrent + ((mNextBit + n + 7)/8) <= mBuffer + mSize)
    {
        for (unsigned i = 0; i < n; i++)
        {
            if (x & (1 << i))
            {
                *mCurrent |= (1 << mNextBit);
            }
            else
            {
                *mCurrent &= ~(1 << mNextBit);
            }
            mNextBit++;
            if (mNextBit > 7)
            {
                mCurrent++;
                mNextBit = 0;
            }
        }
    }
    else
    {
        std::stringstream ss{};
        ss << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " BufferFull!";
        Logging::LogFatal("FileBuffer") << ss.str() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

}
