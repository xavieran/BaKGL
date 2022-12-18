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

#ifndef FILE_BUFFER_H
#define FILE_BUFFER_H

#include <glm/glm.hpp>

#include <array>
#include <fstream>
#include <string>
#include <type_traits>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include "alt_stdint.h"
#endif

#include <ostream>

const unsigned int COMPRESSION_LZW  = 0;
const unsigned int COMPRESSION_LZSS = 1;
const unsigned int COMPRESSION_RLE  = 2;

class FileBuffer
{
    private:
        // Be nicer if this was a shared ptr...
        uint8_t * mBuffer;
        uint8_t * mCurrent;
        unsigned int mSize;
        unsigned int mNextBit;
        bool mOwnBuffer;

        FileBuffer(
            std::uint8_t*,
            std::uint8_t*,
            std::uint32_t,
            std::uint32_t);
            
    public:
        FileBuffer ( const unsigned int n );
        FileBuffer(const FileBuffer&) noexcept = delete;
        FileBuffer& operator=(const FileBuffer&) noexcept = delete;

        FileBuffer(FileBuffer&&) noexcept;
        FileBuffer& operator=(FileBuffer&&) noexcept;

        ~FileBuffer();

        // Create a new file mBuffer starting from this tag
        FileBuffer Find(std::uint32_t tag) const;
        template <typename T>
        FileBuffer Find(T tag) const
        {
            return Find(static_cast<std::uint32_t>(tag));
        }

        void Load ( std::ifstream &ifs );
        void Save ( std::ofstream &ofs );
        void Save ( std::ofstream &ofs, const unsigned int n );
        void Show(std::ostream&);
        void Dump (std::ostream&, const unsigned int n = 0 );
        void Dump ( const unsigned int n = 0 );
        void DumpAndSkip (const unsigned int n = 0 );
        void CopyFrom ( FileBuffer *buf, const unsigned int n );
        void CopyTo ( FileBuffer *buf, const unsigned int n );
        void Fill ( FileBuffer *buf );
        void Rewind();
        unsigned int Tell ();
        void Seek ( const unsigned int n );
        void Skip ( const int n );

        void SkipBits();
        unsigned int CompressLZW ( FileBuffer *result );
        unsigned int CompressLZSS ( FileBuffer *result );
        unsigned int CompressRLE ( FileBuffer *result );
        unsigned int Compress ( FileBuffer *result, const unsigned int method );
        unsigned int DecompressLZW ( FileBuffer *result );
        unsigned int DecompressLZSS ( FileBuffer *result );
        unsigned int DecompressRLE ( FileBuffer *result );
        unsigned int Decompress ( FileBuffer *result, const unsigned int method );

        bool AtEnd() const;
        unsigned int GetSize() const;
        unsigned int GetBytesDone() const;
        unsigned int GetBytesLeft() const;
        uint8_t * GetCurrent() const;
        unsigned int GetNextBit() const;

        uint8_t GetUint8();
        uint16_t GetUint16LE();
        uint16_t GetUint16BE();
        uint32_t GetUint32LE();
        uint32_t GetUint32BE();
        int8_t GetSint8();
        int16_t GetSint16LE();
        int16_t GetSint16BE();
        int32_t GetSint32LE();
        int32_t GetSint32BE();

        template <typename T, bool littleEndian=true>
            requires std::is_integral_v<T>
        T Get()
        {
            if constexpr (std::is_same_v<T, std::uint8_t>)
            {
                return GetUint8();
            }
            else if constexpr (std::is_same_v<T, std::int8_t>)
            {
                return GetSint8();
            }
            else if constexpr (std::is_same_v<T, std::uint16_t>)
            {
                return littleEndian ? GetUint16LE() : GetUint16BE();
            }
            else if constexpr (std::is_same_v<T, std::int16_t>)
            {
                return littleEndian ? GetSint16LE() : GetSint16BE();
            }
            else if constexpr (std::is_same_v<T, std::uint32_t>)
            {
                return littleEndian ? GetUint32LE() : GetUint32BE();
            }
            else if constexpr (std::is_same_v<T, std::int32_t>)
            {
                return littleEndian ? GetSint32LE() : GetSint32BE();
            }
            else
            {
                static_assert(std::is_same_v<T, std::uint8_t>, "Unsupported integral type");
                return T{};
            }
        }

        template <std::size_t N>
        auto GetArray()
        {
            std::array<std::uint8_t, N> arr{};

            for (unsigned i = 0; i < N; i++)
            {
                arr[i] = GetUint8();
            }

            return arr; 
        }

        template <typename T, std::size_t N>
        auto LoadVector()
        {
            glm::vec<N, T> tmp{};
            for (unsigned i = 0; i < N; i++)
            {
                tmp[i] = Get<T>();
            }
            return tmp;
        }

        std::string GetString();
        std::string GetString ( const unsigned int len );
        void GetData ( void * data, const unsigned int n );
        unsigned int GetBits ( const unsigned int n );

        void PutUint8 ( const uint8_t x );
        void PutUint16LE ( const uint16_t x );
        void PutUint16BE ( const uint16_t x );
        void PutUint32LE ( const uint32_t x );
        void PutUint32BE ( const uint32_t x );
        void PutSint8 ( const int8_t x );
        void PutSint16LE ( const int16_t x );
        void PutSint16BE ( const int16_t x );
        void PutSint32LE ( const int32_t x );
        void PutSint32BE ( const int32_t x );
        void PutString ( const std::string s );
        void PutString ( const std::string s, const unsigned int len );
        void PutData ( void * data, const unsigned int n );
        void PutData ( const uint8_t x, const unsigned int n );
        void PutBits ( const unsigned int x, const unsigned int n );
};

class FileBufferFactory
{
public:
    static FileBufferFactory& Get();

    void SetDataPath(const std::string&);
    void SetSavePath(const std::string&);

    FileBuffer CreateDataBuffer(const std::string& path);
    FileBuffer CreateSaveBuffer(const std::string& path);
    FileBuffer CreateFileBuffer(const std::string& path);

private:

    FileBufferFactory();

    FileBufferFactory& operator=(const FileBufferFactory&) noexcept = delete;
    FileBufferFactory(const FileBufferFactory&) noexcept = delete;
    FileBufferFactory& operator=(FileBufferFactory&&) noexcept = delete;
    FileBufferFactory(FileBufferFactory&&) noexcept = delete;

    std::string mDataPath;
    std::string mSavePath;
};

#endif

