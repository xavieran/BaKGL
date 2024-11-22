#include "bak/worldItem.hpp"

#include "bak/constants.hpp"

#include "bak/coordinates.hpp"
#include "bak/file/fileBuffer.hpp"
#include "bak/fileBufferFactory.hpp"

namespace BAK {

auto LoadWorldTile(FileBuffer& fb)
    -> std::pair<std::vector<WorldItem>, glm::ivec2>
{
    glm::ivec2 center{};
    std::vector<WorldItem> items{};

    while (!fb.AtEnd())
    {
        unsigned itemType = fb.GetUint16LE();

        unsigned xrot = fb.GetUint16LE();
        unsigned yrot = fb.GetUint16LE();
        unsigned zrot = fb.GetUint16LE();

        unsigned xloc = fb.GetUint32LE();
        unsigned yloc = fb.GetUint32LE();
        unsigned zloc = fb.GetUint32LE();

        if (itemType == 0)
        {
            center = {xloc, yloc};
        }

        items.emplace_back(
            WorldItem{
                itemType,
                glm::ivec3{xrot, yrot, zrot},
                glm::ivec3{xloc, yloc, zloc}});
    }

    return std::make_pair(items, center);
}

}
