/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

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

#include "SDL_endian.h"

#include "Defines.h"
#include "Exception.h"
#include "FileBuffer.h"

unsigned GetStreamSize(std::ifstream& ifs)
{
    ifs.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = ifs.gcount();
    ifs.clear();
    ifs.seekg( 0, std::ios_base::beg );
    return static_cast<unsigned>(length);
}

FileBufferFactory::FileBufferFactory()
:
    mDataPath{(std::filesystem::path{GetBakDirectory()} / "data").string()},
    mSavePath{(std::filesystem::path{GetBakDirectory()} / "save").string()}
{}

FileBufferFactory& FileBufferFactory::Get()
{
    static FileBufferFactory factory{};
    return factory;
}

void FileBufferFactory::SetDataPath(const std::string& dataPath)
{
    mDataPath = dataPath;
}

void FileBufferFactory::SetSavePath(const std::string& savePath)
{
    mSavePath = savePath;
}


FileBuffer FileBufferFactory::CreateDataBuffer(const std::string& path)
{
    // First look in the cwd
    if (std::filesystem::exists(path))
        return CreateFileBuffer(path);
    else
    {
        const auto realPath = std::filesystem::path{mDataPath} / path;
        return CreateFileBuffer(realPath.string());
    }
}

FileBuffer FileBufferFactory::CreateSaveBuffer(const std::string& path)
{
    if (std::filesystem::exists(path))
        return CreateFileBuffer(path);
    else
    {
        const auto realPath = std::filesystem::path{mSavePath} / path;
        return CreateFileBuffer(realPath.string());
    }
}

FileBuffer FileBufferFactory::CreateFileBuffer(const std::string& path)
{
    Logging::LogInfo(__FUNCTION__) << "Opening: " << path << std::endl;
    std::ifstream in{};
    in.open(path, std::ios::in | std::ios::binary);
    if (!in.good())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        throw OpenError(__FILE__, __LINE__);
    }

    FileBuffer fb{GetStreamSize(in)};
    fb.Load(in);
    in.close();
    return fb;
}

FileBuffer::FileBuffer(
    std::uint8_t* buf,
    std::uint8_t* cur,
    std::uint32_t sz,
    std::uint32_t nb)
:
    mBuffer{buf},
    mCurrent{cur},
    mSize{sz},
    mNextBit{nb},
    mOwnBuffer{false}
{
}

FileBuffer::FileBuffer(const unsigned int n)
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
FileBuffer::CopyFrom(FileBuffer *buf, const unsigned int n)
{
    if (mBuffer && n && (mCurrent + n <= mBuffer + mSize))
    {
        buf->GetData(mCurrent, n);
        mCurrent += n;
    }
}

void
FileBuffer::CopyTo(FileBuffer *buf, const unsigned int n)
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
        buf->GetData(mBuffer, MIN(mSize, buf->GetSize()));
    }
}

FileBuffer FileBuffer::Find(std::uint32_t tag) const
{
    auto *search = mBuffer;
    for (; search < (mBuffer + mSize - sizeof(std::uint32_t)); search++)
    {
        auto cur = *reinterpret_cast<std::uint32_t*>(search);
        if (cur == tag)
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

void
FileBuffer::Load(std::ifstream &ifs)
{
    if (ifs.is_open())
    {
        mCurrent = mBuffer;
        ifs.read((char *)mBuffer, mSize);
        if (ifs.fail())
        {
            throw IOError(__FILE__, __LINE__);
        }
    }
    else
    {
        throw OpenError(__FILE__, __LINE__);
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
            throw IOError(__FILE__, __LINE__);
        }
    }
    else
    {
        throw OpenError(__FILE__, __LINE__);
    }
}

void
FileBuffer::Save(std::ofstream &ofs, const unsigned int n)
{
    if (ofs.is_open())
    {
        if (n <= mSize)
        {
            mCurrent = mBuffer;
            ofs.write((char *)mBuffer, n);
            if (ofs.fail())
            {
                throw IOError(__FILE__, __LINE__);
            }
        }
        else
        {
            throw BufferEmpty(__FILE__, __LINE__);
        }
    }
    else
    {
        throw OpenError(__FILE__, __LINE__);
    }
}


void
FileBuffer::Dump(const unsigned int n)
{
    if (n == 0) return;
    Dump(std::cout, n);
}

void
FileBuffer::DumpAndSkip(const unsigned int n)
{
    if (n == 0) return;
    Dump(n);
    Skip(n);
}

void
FileBuffer::Dump(std::ostream& os, const unsigned int n)
{
    uint8_t* tmp = mCurrent;
    unsigned int count = 0;
    os << std::setbase(16) << std::setfill('0') << std::setw(8) << count << ": ";
    while ((tmp < (mBuffer + mSize)) && ((tmp < (mCurrent + n)) || (n == 0)))
    {
        os << std::setw(2) << (unsigned int)*tmp++ << " ";
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
FileBuffer::Seek(const unsigned int n)
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

unsigned int
FileBuffer::CompressLZW(FileBuffer *result)
{
    try
    {
        std::map<uint32_t, uint16_t> hashtable;
        unsigned int n_bits = 9;
        unsigned int free_entry = 257;
        unsigned int bitpos = 0;
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
                if (free_entry >= (unsigned int)(1 << n_bits))
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
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::CompressLZW");
        throw;
    }
    return 0;
}

unsigned int
FileBuffer::CompressLZSS(FileBuffer *result)
{
    try
    {
        uint8_t *data = GetCurrent();
        uint8_t *curr = GetCurrent();
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
            unsigned int off = 0;
            unsigned int len = 0;
            uint8_t *ptr = curr;
            while (ptr > data)
            {
                ptr--;
                if (*ptr == byte)
                {
                    off = ptr - data;
                    len = 1;
                    while ((curr + len < mBuffer + mSize) && (ptr[len] == curr[len]))
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
            curr = GetCurrent();
            byte = GetUint8();
            mask <<= 1;
        }
        *codeptr = code;
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::CompressLZSS");
        throw;
    }
    return 0;
}

unsigned int
FileBuffer::CompressRLE(FileBuffer *result)
{
    try
    {
        uint8_t *skipptr = GetCurrent();
        uint8_t byte = 0;
        uint8_t next = GetUint8();
        unsigned int count;
        unsigned int skipped = 0;
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
                        unsigned int n;
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
                    unsigned int n;
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
                unsigned int n = skipped & 0x7f;
                result->PutUint8(n);
                result->CopyFrom(this, n);
                skipped -= n;
            }
        }
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::CompressRLE");
        throw;
    }
    return 0;
}

unsigned int
FileBuffer::Compress(FileBuffer *result, const unsigned int method)
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
        throw CompressionError(__FILE__, __LINE__);
        break;
    }
}

typedef struct _CodeTableEntry
{
    uint16_t prefix;
    uint8_t  append;
}
CodeTableEntry;

unsigned int
FileBuffer::DecompressLZW(FileBuffer *result)
{
    try
    {
        CodeTableEntry *codetable = new CodeTableEntry[4096];
        uint8_t *decodestack = new uint8_t[4096];
        uint8_t *stackptr = decodestack;
        unsigned int n_bits = 9;
        unsigned int free_entry = 257;
        unsigned int oldcode = GetBits(n_bits);
        unsigned int lastbyte = oldcode;
        unsigned int bitpos = 0;
        result->PutUint8(oldcode);
        while (!AtEnd() && !result->AtEnd())
        {
            unsigned int newcode = GetBits(n_bits);
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
                unsigned int code = newcode;
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
                    if ((free_entry >= (unsigned int)(1 << n_bits)) && (n_bits < 12))
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
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::DecompressLZW");
        throw;
    }
    return 0;
}

unsigned int
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
                unsigned int off = GetUint16LE();
                unsigned int len = GetUint8() + 5;
                result->PutData(data + off, len);
            }
            mask <<= 1;
        }
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::DecompressLZSS");
        throw;
    }
    return 0;
}

unsigned int
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
        unsigned int res = result->GetBytesDone();
        result->Rewind();
        return res;
    }
    catch (Exception &e)
    {
        e.Print("FileBuffer::DecompressRLE");
        throw;
    }
    return 0;
}

unsigned int
FileBuffer::Decompress(FileBuffer *result, const unsigned int method)
{
    switch (method)
    {
    case COMPRESSION_LZW:
        if ((GetUint8() != 0x02) || (GetUint32LE() != result->GetSize()))
        {
            throw DataCorruption(__FILE__, __LINE__);
        }
        return DecompressLZW(result); break;
    case COMPRESSION_LZSS:
        return DecompressLZSS(result);
        break;
    case COMPRESSION_RLE:
        return DecompressRLE(result);
        break;
    default:
        throw CompressionError(__FILE__, __LINE__);
        break;
    }
}

bool
FileBuffer::AtEnd() const
{
    return (mCurrent >= mBuffer + mSize);
}

unsigned int
FileBuffer::GetSize() const
{
    return mSize;
}

unsigned int
FileBuffer::GetBytesDone() const
{
    return (mCurrent - mBuffer);
}

unsigned int
FileBuffer::GetBytesLeft() const
{
    return (mBuffer + mSize - mCurrent);
}

uint8_t *
FileBuffer::GetCurrent() const
{
    return mCurrent;
}

unsigned int
FileBuffer::GetNextBit() const
{
    return mNextBit;
}

void
FileBuffer::Rewind()
{
    mCurrent = mBuffer;
}

unsigned int
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
            throw BufferEmpty(__FILE__, __LINE__);
        }
    }
    return "";
}

std::string
FileBuffer::GetString(const unsigned int len)
{
    if ((mCurrent) && (mCurrent + len <= mBuffer + mSize))
    {
        std::string s((char *)mCurrent);
        mCurrent += len;
        return s;
    }
    else
    {
        throw BufferEmpty(__FILE__, __LINE__);
    }
    return "";
}

void
FileBuffer::GetData(void *data,
                    const unsigned int n)
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
        throw BufferEmpty(__FILE__, __LINE__);
    }
}

unsigned int
FileBuffer::GetBits(const unsigned int n)
{
    if (mCurrent + ((mNextBit + n + 7)/8) <= mBuffer + mSize)
    {
        unsigned int x = 0;
        for (unsigned int i = 0; i < n; i++)
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
        throw BufferEmpty(__FILE__, __LINE__);
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
        throw BufferFull(__FILE__, __LINE__);
    }
}

void
FileBuffer::PutString(const std::string s, const unsigned int len)
{
    if ((mCurrent) && (mCurrent + len <= mBuffer + mSize))
    {
        memset(mCurrent, 0, len);
        strncpy((char *)mCurrent, s.c_str(), len);
        mCurrent += len;
    }
    else
    {
        throw BufferFull(__FILE__, __LINE__);
    }
}

void
FileBuffer::PutData(void *data, const unsigned int n)
{
    if (mCurrent + n <= mBuffer + mSize)
    {
        memcpy(mCurrent, data, n);
        mCurrent += n;
    }
    else
    {
        throw BufferFull(__FILE__, __LINE__);
    }
}

void FileBuffer::PutData(const uint8_t x, const unsigned int n)
{
    if (mCurrent + n <= mBuffer + mSize)
    {
        memset(mCurrent, x, n);
        mCurrent += n;
    }
    else
    {
        throw BufferFull(__FILE__, __LINE__);
    }
}

void FileBuffer::PutBits(const unsigned int x, const unsigned int n)
{
    if (mCurrent + ((mNextBit + n + 7)/8) <= mBuffer + mSize)
    {
        for (unsigned int i = 0; i < n; i++)
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
        throw BufferFull(__FILE__, __LINE__);
    }
}
