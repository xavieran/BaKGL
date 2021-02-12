#pragma once

#include "constants.hpp"
#include "logger.hpp"

#include "FileBuffer.h"

#include <unordered_map>
#include <cassert>

namespace BAK {

class Keywords
{
public:
    void Load(FileBuffer& fb)
    {
        auto length = fb.GetUint16LE();
        std::cout << "Loading keywords" << std::endl;
        std::cout << "Length: " << length << std::endl;

        unsigned i = 0;
        while (fb.Tell() != 0x2b8)
        {
            std::cout << "I: " << i << " " << std::hex << fb.GetUint16LE() << std::dec << std::endl;
            i++;
        }
        i = 0;
        while (fb.GetBytesLeft() != 0)
        {
            std::cout << "Str:" << i << " :: " << fb.GetString() << std::endl;
            i++;
        }
    }
};

class ParentDialog
{
public:
    using ChapterType = unsigned;

    ParentDialog()
    :
        mChapterMap{},
        mLogger{Logging::LogState::GetLogger("ParentDialog")}
    {}
    
    // fb starts pointing in the right spot
    void Load(FileBuffer& fb)
    {
        ChapterType chapter = 1;

        fb.Dump(64);
        fb.GetUint32LE(); // These are various flags determining style properties
        fb.Dump(1);
        fb.Skip(1);
        unsigned dialogType = fb.GetUint8();
        fb.Dump(1);
        fb.Skip(1);
        mLogger.Info() << "Subdialogues: " << dialogType << std::endl;
        dialogType--;
        fb.Dump(2);
        fb.Skip(2);

        for (unsigned i = 0; i < dialogType; i++)
        {
            assert(fb.GetUint16LE() == 0x7537);
            auto chapterPrimary   = fb.GetUint16LE();
            auto chapterSecondary = fb.GetUint16LE();
            unsigned dialogOffset = fb.GetUint32LE();
            mLogger.Info() << "Chapters: " << chapterPrimary << " " 
                << chapterSecondary << " -> " << std::hex
                << dialogOffset << std::dec << std::endl;
            mChapterMap.emplace(chapterPrimary, dialogOffset);
            mChapterMap.emplace(chapterSecondary, dialogOffset);
        }

        chapter = 1;
        mLogger.Info() << "Loading Chapter " << chapter 
            << " @ 0x" << std::hex << mChapterMap[chapter] 
            << std::dec << std::endl;
        fb.Seek(mChapterMap[chapter]);
        
        fb.Dump(32);
        
        auto style = fb.GetUint32LE();
        fb.Dump(1);
        fb.Skip(1);
        fb.Dump(8);
        dialogType = fb.GetUint8();
        auto extraBlocks = fb.GetUint8();
        mLogger.Info() << "DialogType: " << dialogType << std::endl;
        auto length = fb.GetUint16LE();
        auto randomPtr = fb.GetUint32LE();
        //assert(fb.GetUint16LE() == 0xffff);
        auto alternateDialogPtr = fb.GetUint32LE();
        std::cout << "RandomPtr: " << std::hex << randomPtr << " altDialog: " << alternateDialogPtr << std::endl;
        fb.Dump(6);
        fb.Skip(6);
        auto offset = fb.GetUint32LE();
        std::cout << " st: " << std::hex << style << std::dec 
            << " diagType: " << +dialogType << " length: " 
            << +length << " extra: " << +extraBlocks << " next: " 
            << std::hex << offset << std::endl;
        fb.Dump(extraBlocks * 10);
        fb.Skip(extraBlocks * 10);

        fb.Seek(offset);
        style = fb.GetUint32LE();
        fb.Dump(1);
        fb.Skip(1);
        dialogType = fb.GetUint8();
        extraBlocks = fb.GetUint8();
        length = fb.GetUint16LE();
        fb.Dump(6);
        fb.Skip(6);
        offset = fb.GetUint32LE();
        std::cout << " st: " << std::hex << style << std::dec 
            << " diagType: " << dialogType << " length: " 
            << length << " extra: " << +extraBlocks << " next: " 
            << std::hex << offset << std::endl;
        fb.Dump(extraBlocks * 10);
        fb.Skip(extraBlocks * 10);

        fb.Seek(offset);

        style = fb.GetUint32LE();
        fb.Dump(1);
        fb.Skip(1);
        dialogType = fb.GetUint8();
        extraBlocks = fb.GetUint8();
        length = fb.GetUint16LE();
        fb.Dump(6);
        fb.Skip(6);
        offset = fb.GetUint32LE();
        std::cout << " st: " << std::hex << style << std::dec 
            << " diagType: " << +dialogType << " length: " 
            << +length << " extra: " << +extraBlocks << " next: " 
            << std::hex << offset << std::endl;
        fb.Dump(extraBlocks * 10);
        fb.Skip(extraBlocks * 10);

        std::cout << "Contents [" << fb.GetString(length) << "]" << std::endl;

        const auto NextDialog = [&](){
            fb.Seek(offset);
            fb.Dump(1);
            fb.Skip(1);
            auto actor = fb.GetUint16LE();
            std::cout << "Actor: " << std::dec << actor << std::endl;
            fb.Dump(2);
            fb.Skip(2);
            dialogType = fb.GetUint8();
            extraBlocks = fb.GetUint8();
            if (dialogType == 2) extraBlocks++;
            length = fb.GetUint16LE();
            std::cout << " diagType: " << +dialogType << " extraBlocks: " 
                << +extraBlocks << "Length: " << length << std::endl;
            fb.Dump(6);
            fb.Skip(6);
            offset = fb.GetUint32LE();
            if (offset & 0x80000000)
            {
                std::cout << " ** Tree key: " << std::hex 
                    << (offset & 0x7fffffff) << std::dec << std::endl;
                fb.Dump(2);
                fb.Skip(2);
            offset = fb.GetUint32LE();
            }
            else
            {
                fb.Dump(extraBlocks * 10);
                fb.Skip(extraBlocks * 10);
            }

            std::cout << "Contents [" << fb.GetString(length) << "]" << std::endl;
            std::cout << "Next Dialog: " << std::hex << offset << std::endl;
        };
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
        NextDialog();
    }

private:
    using OffsetType = unsigned;

    std::unordered_map<ChapterType, OffsetType> mChapterMap;
    const Logging::Logger& mLogger;
};

class Dialog
{
public:
    using KeyType = unsigned;

    Dialog()
    :
        mDialogMap{},
        mLogger{Logging::LogState::GetLogger("Dialog")}
    {}

    void LoadKeys()
    {
        for (std::uint8_t dialogFile = 0; dialogFile < 32; dialogFile++)
        {
            auto fname = GetDialogFile(dialogFile);
            auto fb = FileBufferFactory::CreateFileBuffer(fname);
            unsigned dialogs = fb.GetUint16LE();
            mLogger.Info() << "Dialog " << fname << " has: " << dialogs << " dialogs" << std::endl;

            for (unsigned i = 0; i < dialogs; i++)
            {
                auto key = fb.GetUint32LE();
                auto val = fb.GetUint32LE();
                const auto [it, emplaced] = mDialogMap.emplace(
                    key,
                    std::make_pair(dialogFile, val));
                assert(emplaced);
                auto [checkF, checkV] = it->second;
                mLogger.Spam() << std::hex << "0x" << it->first 
                    << " -> 0x" << checkV << std::dec << std::endl;
            }
        }
    }

    void ShowDialog(KeyType dialogKey)
    {
        auto it = mDialogMap.find(dialogKey);
        if (it == mDialogMap.end()) throw std::runtime_error("Key not found");
        auto [dialogFile, offset] = it->second;
        mLogger.Info() << "Loading dialog@ 0x" << std::hex 
            << offset << std::dec << " from: " << +dialogFile << std::endl;

        auto fbo = FileBufferFactory::CreateFileBuffer(GetDialogFile(dialogFile));
        fbo.Seek(offset);

        ParentDialog p{};
        p.Load(fbo);
    }

    std::string GetDialogFile(std::uint8_t i)
    {
        std::stringstream ss{};
        ss << "DIAL_Z" << std::setw(2) << std::setfill('0') << +i << ".DDX";
        return ss.str();
    }

private:
    using ValueType = std::pair<std::uint8_t, unsigned>;

    std::unordered_map<KeyType, ValueType> mDialogMap;
    const Logging::Logger& mLogger;
};

}
