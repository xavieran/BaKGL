#pragma once

#include <glm/glm.hpp>

#include <array>
#include <fstream>
#include <string>
#include <type_traits>

#include <cstdint>
#include <ostream>

namespace BAK {

static constexpr auto COMPRESSION_LZW  = 0;
static constexpr auto COMPRESSION_LZSS = 1;
static constexpr auto COMPRESSION_RLE  = 2;

class FileBuffer
{
private:
    // Be nicer if this was a shared ptr...
    std::uint8_t * mBuffer;
    std::uint8_t * mCurrent;
    unsigned mSize;
    unsigned mNextBit;
    bool mOwnBuffer;

    FileBuffer(
        std::uint8_t*,
        std::uint8_t*,
        std::uint32_t,
        std::uint32_t);
        
public:
    explicit FileBuffer(unsigned n);
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

    void Load(std::ifstream &ifs);
    void Save(std::ofstream &ofs);
    void Save(std::ofstream &ofs, unsigned n);
    void Show(std::ostream&);
    void Dump(std::ostream&, unsigned n = 0);
    void Dump(unsigned n = 0);
    void DumpAndSkip(unsigned n = 0);
    void CopyFrom(FileBuffer *buf, unsigned n);
    void CopyTo(FileBuffer *buf, unsigned n);
    void Fill(FileBuffer *buf);
    void Rewind();
    unsigned Tell();
    void Seek(unsigned n);
    void Skip(int n);

    void SkipBits();
    unsigned CompressLZW(FileBuffer *result);
    unsigned CompressLZSS(FileBuffer *result);
    unsigned CompressRLE(FileBuffer *result);
    unsigned Compress(FileBuffer *result, unsigned method);
    unsigned DecompressLZW(FileBuffer *result);
    unsigned DecompressLZSS(FileBuffer *result);
    unsigned DecompressRLE(FileBuffer *result);
    unsigned Decompress(FileBuffer *result, unsigned method);

    bool AtEnd() const;
    unsigned GetSize() const;
    unsigned GetBytesDone() const;
    unsigned GetBytesLeft() const;
    std::uint8_t * GetCurrent() const;
    unsigned GetNextBit() const;

    std::uint8_t GetUint8();
    std::uint16_t GetUint16LE();
    std::uint16_t GetUint16BE();
    std::uint32_t GetUint32LE();
    std::uint32_t GetUint32BE();
    std::int8_t GetSint8();
    std::int16_t GetSint16LE();
    std::int16_t GetSint16BE();
    std::int32_t GetSint32LE();
    std::int32_t GetSint32BE();

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
    std::string GetString(unsigned len);
    void GetData(void * data, unsigned n);
    unsigned GetBits(unsigned n);

    void PutUint8(uint8_t x);
    void PutUint16LE(std::uint16_t x);
    void PutUint16BE(std::uint16_t x);
    void PutUint32LE(std::uint32_t x);
    void PutUint32BE(std::uint32_t x);
    void PutSint8(std::int8_t x);
    void PutSint16LE(std::int16_t x);
    void PutSint16BE(std::int16_t x);
    void PutSint32LE(std::int32_t x);
    void PutSint32BE(std::int32_t x);
    void PutString(std::string s);
    void PutString(std::string s, unsigned len);
    void PutData(void * data, unsigned n);
    void PutData(std::uint8_t x, unsigned n);
    void PutBits(unsigned x, unsigned n);
};

class FileBufferFactory
{
public:
    static FileBufferFactory& Get();

    void SetDataPath(const std::string&);
    void SetSavePath(const std::string&);

    bool DataBufferExists(const std::string& path);
    FileBuffer CreateDataBuffer(const std::string& path);
    bool SaveBufferExists(const std::string& path);
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

}
