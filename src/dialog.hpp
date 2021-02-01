#pragma once

#include "constants.hpp"
#include "logger.hpp"

#include "FileBuffer.h"

#include <unordered_map>
#include <cassert>

namespace BAK {

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
        assert(fb.GetUint32LE() == 0x10000000);
        fb.Dump(1);
        fb.Skip(1);
        unsigned subDialogues = fb.GetUint16LE() - 1;
        mLogger.Info() << "Subdialogues: " << subDialogues << std::endl;
        fb.Dump(2);
        fb.Skip(2);

        for (unsigned i = 0; i < subDialogues; i++)
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

        ChapterType chapter = 1;
        fb.Seek(mChapterMap[chapter]);
        fb.Dump(32);

        assert(fb.GetUint32LE() == 0x10000000);
        fb.Dump(1);
        fb.Skip(1);
        unsigned dialogueType = fb.GetUint16LE();
        mLogger.Info() << "DialogType: " << dialogueType << std::endl;
        fb.Dump(2);
        fb.Skip(2);

        auto startPtr = fb.GetUint32LE();
        assert(fb.GetUint16LE() == 0xffff);
        auto endPtr = fb.GetUint32LE();
        std::cout << "Start: " << std::hex << startPtr << " End: " << endPtr << std::endl;
        fb.Dump(6);
        fb.Skip(6);
        std::cout << "Offset: " << std::hex << fb.GetUint32LE() << std::endl;
        assert(fb.GetUint32LE() == 0x10000000);
        fb.Dump(1);
        fb.Skip(1);
        fb.Dump(2);
        fb.Skip(2);
        fb.Dump(8);
        fb.Skip(8);
        auto offset = fb.GetUint32LE();
        std::cout << "Offset: " << std::hex << offset << std::endl;
        fb.Seek(offset);
        //assert(fb.GetUint32LE() == 0x0411000c);
        fb.Dump(1);
        fb.Skip(1);
        auto actor = fb.GetUint16LE();
        std::cout << "Actor: " << actor << std::endl;
        fb.Dump(4);
        fb.Skip(4);
        auto length = fb.GetUint16LE();
        std::cout << "Length: " << length << std::endl;
        fb.Dump(6);
        fb.Skip(6);
        offset = fb.GetUint32LE();
        std::cout << "Offset2: " << std::hex << offset << std::endl;

        fb.Dump(8);
        fb.Skip(8);
        auto nextDialog = offset;
        std::cout << "Next Dialog: " << std::hex << nextDialog << std::endl;
        fb.Dump(20);
        fb.Skip(20);
        std::cout << "Contents [" << fb.GetString(length) << "]" << std::endl;

        const auto NextDialog = [&](){
            fb.Seek(nextDialog);
            fb.Dump(20);
            fb.Dump(1);
            fb.Skip(1);
            actor = fb.GetUint16LE();
            std::cout << "Actor: " << std::dec << actor << std::endl;
            fb.Dump(4);
            fb.Skip(4);
            length = fb.GetUint16LE();
            std::cout << "Length: " << length << std::endl;
            fb.Dump(6);
            fb.Skip(6);
            nextDialog = fb.GetUint32LE();
            std::cout << "Contents [" << fb.GetString(length) << "]" << std::endl;
            std::cout << "Next Dialog: " << std::hex << nextDialog << std::endl;
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

    void Load(FileBuffer& fb, KeyType dialogKey)
    {
        unsigned dialogs = fb.GetUint16LE();
        mLogger.Info() << "Dialog has: " << dialogs << " dialogs" << std::endl;

        for (unsigned i = 0; i < dialogs; i++)
        {
            auto key = fb.GetUint32LE();
            auto val = fb.GetUint32LE();
            const auto [it, emplaced] = mDialogMap.emplace(key, val);
            mLogger.Debug() << std::hex << "0x" << it->first 
                << " -> 0x" << it->second << std::dec << std::endl;
        }
        
        mLogger.Info() << "Loading dialog: " << std::hex 
            << dialogKey << std::dec << std::endl;
        fb.Seek(mDialogMap[dialogKey]);

        ParentDialog p{};
        p.Load(fb);
    }

private:
    using OffsetType = unsigned;

    std::unordered_map<KeyType, OffsetType> mDialogMap;
    const Logging::Logger& mLogger;
};

}
