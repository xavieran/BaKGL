#include "src/scene.hpp"

#include "src/logger.hpp"

#include "xbak/ResourceTag.h"
#include "xbak/TaggedResource.h"


#include <cassert>
#include <cctype>
#include <functional>

namespace BAK {

std::vector<Scene> LoadScenes(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<Scene> scenes{};

    fb.DumpAndSkip(20);

    auto pageBuffer    = fb.Find(TAG_PAG);
    auto versionBuffer = fb.Find(TAG_VER);
    auto tt3Buffer     = fb.Find(TAG_TT3);
    auto tagBuffer     = fb.Find(TAG_TAG);
    
    const auto pages = pageBuffer.GetUint16LE();
    logger.Debug() << "Pages:" << pages << "\n";

    tt3Buffer.Skip(1);
    FileBuffer decompBuffer = FileBuffer(tt3Buffer.GetUint32LE());
    auto decomped = tt3Buffer.DecompressRLE(&decompBuffer);
    std::cout << "Decompressed size:" << decomped << "\n";
    decompBuffer.Dump(decomped);
    ResourceTag tags;
    tags.Load(&tagBuffer);

    for (const auto& [id, tag] : tags.GetTagMap())
        std::cout << "Id: " << id << " tag: " << tag << "\n";

    std::cout << "Loading movie chunks" << std::endl;

    while (!decompBuffer.AtEnd())
    {
        unsigned int code = decompBuffer.GetUint16LE();
        unsigned int size = code & 0x000f;
        code &= 0xfff0;
        logger.Debug() << "Code: " << std::hex << code << " " 
            << static_cast<Actions>(code) << " sz: "
            << std::dec << size << "\n";
        
        if ((code == 0x1110) && (size == 1))
        {
            unsigned int id = decompBuffer.GetUint16LE();
            std::string name;
            if (tags.Find(id, name))
            {
                logger.Debug() << "Name: " << name <<"\n";
            }
        }
        else if (size == 0xf)
        {
            auto name = decompBuffer.GetString();
            //std::transform(name.begin(), name.end(), name.begin(), std::toupper);
            logger.Debug() << "Name: " << name <<"\n";
            if (decompBuffer.GetBytesLeft() & 1)
                decompBuffer.Skip(1);
        }
        else
        {
            std::stringstream ss{};
            for (unsigned int i = 0; i < size; i++)
                ss << " " << decompBuffer.GetSint16LE();
            logger.Debug() << ss.str() << "\n";

               //mc->data.push_back(decompBuffer.GetSint16LE());
        }

    }

    //for (const auto& scene : scenes)
    //    std::cout << "Scene: " << scene.mScene << "\n";

    return scenes;
}

}
