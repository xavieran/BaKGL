#pragma once

#include "constants.hpp"
#include "dialog.hpp"
#include "resourceNames.hpp"
#include "logger.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

namespace BAK {

// Loaded from a GDS File
class Scene
{
public:
    void Load(FileBuffer& fb)
    {
        BAK::DialogStore dialogStore{};
        dialogStore.Load();
        auto length = fb.GetUint16LE();
        std::cout << "Length: " << length << std::endl;
        auto name = fb.GetString(6);
        std::cout << "Name: " << name << std::endl;
        fb.Skip(4);
        fb.Skip(2);
        std::cout << "XX: " << std::hex << fb.GetUint16LE() 
            << " YY: " << fb.GetUint16LE() << std::endl;
        fb.Dump(4); // Some kind of addr?
        fb.Skip(4);
        fb.Dump(2); // Not sure
        fb.Skip(2);
        fb.Dump(5); // Inn background/people identifier
        fb.Skip(5);
        auto numHotSpots = fb.GetUint16LE(); 
        std::uint32_t flavourText = fb.GetUint32LE(); 
        std::cout << "Hotspots: " << std::dec << numHotSpots << std::endl;
        std::cout << "Flavour Text: " << std::hex << flavourText << std::endl;
        auto snip = dialogStore.GetSnippet(KeyTarget{flavourText});
        std::cout << snip.GetText() << std::endl;
        fb.Dump(4);
        fb.Skip(4);
        fb.Dump(4);
        fb.Skip(4);
        for (unsigned i = 0; i < numHotSpots; i++)
        {
            auto x = fb.GetUint16LE();
            auto y = fb.GetUint16LE();
            auto w = fb.GetUint16LE();
            auto h = fb.GetUint16LE();
            std::cout << "Hotspot #" << std::dec << i << std::endl;
            std::cout << "coords: " << std::dec << x << " " << y
                << " " << w << " " << h << std::endl;
            fb.Dump(2); // Seems to have some effect...
            fb.Skip(2);
            auto keyword = fb.GetUint16LE();
            std::cout << "Kw: " << keyword << std::endl;
            auto action = fb.GetUint16LE();
            std::cout << "Action: " << action << std::endl;
            fb.Dump(4); // Seems to have some effect...
            fb.Skip(4);
            fb.Dump(4); // Seems to have some effect...
            fb.Skip(4);
            {
            std::uint32_t text = fb.GetUint32LE(); 
            auto snip2 = dialogStore.GetSnippet(KeyTarget{text});
            std::cout << "RightClick: " << std::hex << text << snip2.GetText() << std::endl;
            }
            fb.Dump(4); // Seems to have some effect...
            fb.Skip(4);
            {
            std::uint32_t text = fb.GetUint32LE(); 
            std::cout << "LeftClick: " << std::hex << text << std::endl;
            if (text != 0 && text != 0x10000)
            {
                auto snip2 = dialogStore.GetSnippet(KeyTarget{text});
                std::cout << snip2.GetText() << std::endl;
            }
            }
            fb.Skip(2);
        }
    }
};

}
