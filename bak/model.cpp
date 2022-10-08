#include "bak/model.hpp"

#include "com/ostream.hpp"
#include "xbak/TaggedResource.h"

namespace BAK {

std::vector<std::string> LoadModelNames(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    
    fb.Skip(2);
    const auto numItems = fb.GetUint16LE();

    std::vector<unsigned> offsets{};
    std::vector<std::string> mapItems{};
    offsets.reserve(numItems);
    mapItems.reserve(numItems);

    for (unsigned i = 0; i < numItems; i++)
    {
        offsets.emplace_back(fb.GetUint16LE());
    }

    fb.Skip(2);
    const auto dataStart = fb.GetBytesDone();

    for (auto offset : offsets)
    {
        fb.Seek(dataStart + offset);
        mapItems.emplace_back(fb.GetString());
    }

    return mapItems;
}

unsigned CalculateOffset(unsigned upper, unsigned lower)
{
    return (upper << 4) + (lower & 0xf);
}

std::vector<ModelClip> LoadModelClip(FileBuffer& fb, unsigned numItems)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<unsigned> offsets{};
    offsets.reserve(numItems);
    std::vector<ModelClip> modelClips{};
    modelClips.reserve(numItems);

    for (unsigned i = 0; i < numItems; i++)
    {
        const auto lower = fb.GetUint16LE();
        const auto upper = fb.GetUint16LE();
        
        offsets.emplace_back(CalculateOffset(upper, lower));
    }

    for (unsigned int i = 0; i < numItems; i++)
    {
        fb.Seek(offsets[i]); 
        std::vector<glm::ivec2> textureCoords{};
        std::vector<glm::ivec2> otherCoords{};

        const auto xradius = fb.GetUint16LE();
        const auto yradius = fb.GetUint16LE();
        logger.Debug() << "HitRad: (" << xradius << "," << yradius << ")\n";

        const bool more = fb.GetUint16LE() > 0;
        const auto flags = fb.GetUint16LE();

        if (more)
        {
            // This is clearly wrong...
            fb.Skip(2);
            const auto n = fb.GetUint16LE();
            fb.Skip(2);
            for (unsigned component = 0; component < n; component++)
            {
                const auto u = fb.GetSint8();
                const auto v = fb.GetSint8();
                const auto x = fb.GetSint16LE();
                const auto y = fb.GetSint16LE();
                logger.Debug() << " uvxy " << u << " " << v << " " << x << " " << y << std::endl; 
                textureCoords.emplace_back(u, v);
                otherCoords.emplace_back(x, y);
            }
        }
        modelClips.emplace_back(
            ModelClip{
                xradius, yradius,
                flags, 0,
                0,
                std::move(textureCoords),
                std::move(otherCoords)}
        );
    }

    return modelClips;
}

std::vector<Model> LoadModels(FileBuffer& fb, std::vector<std::string> itemNames)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    const unsigned numItems = itemNames.size();
    std::vector<unsigned> offsets{};
    offsets.reserve(numItems);
    std::vector<Model> models{};
    models.reserve(numItems);

    for (unsigned i = 0; i < numItems; i++)
    {
        const auto lower = fb.GetUint16LE();
        const auto upper = fb.GetUint16LE();
        offsets.emplace_back(CalculateOffset(upper, lower));
    }

    for (unsigned i = 0; i < numItems; i++)
    {
        Model model{};

        fb.Seek(offsets[i]);
        model.mName = itemNames[i];
        model.mEntityFlags = fb.GetUint8();
        model.mEntityType = fb.GetUint8();
        model.mTerrainType = fb.GetUint8();
        model.mScale = fb.GetUint8();
        fb.Skip(4);
        const bool more = fb.GetUint16LE() > 0;
        fb.Skip(4);

        if (more)
        {
            if (!(model.mEntityFlags & EF_UNBOUNDED))
            {
                model.mMin = fb.LoadVector<std::int16_t, 3>();
                model.mMax = fb.LoadVector<std::int16_t, 3>();
            }
            fb.Skip(2);

            unsigned nComponents = fb.GetUint16LE();
            fb.Skip(2); // Seems important...
            unsigned nVertices = 0;
            unsigned prevVertex = 0;
            // Which component do we go over the 255 vertex limit?
            unsigned changeOverComponent = 0;
            unsigned changeOverVerticesOffset = 0;
            for (unsigned component = 0; component < nComponents; component++)
            {
                fb.Skip(3);
                unsigned v = fb.GetUint8();
                if (v != prevVertex)
                {
                    nVertices += v;
                    changeOverComponent = component;
                    changeOverVerticesOffset = prevVertex;
                    prevVertex = v;
                }
                fb.Skip(10);
            }

            if (   model.mEntityType == 0x0
                || model.mEntityType == 0x1
                || model.mEntityType == 0x2
                || model.mEntityType == 0x3
                || model.mEntityType == 0x4
                || model.mEntityType == 0x5
                || model.mEntityType == 0x6
                || model.mEntityType == 0x7
                || model.mEntityType == 0x8
                || model.mEntityType == 0x9
                || model.mEntityType == 0xa
                || model.mEntityType == 0xe
                || model.mEntityType == 0xf
                || model.mEntityType == 0x12
                || model.mEntityType == 0x14
                || model.mEntityType == 0x17
                || model.mEntityType == 0x24
                || model.mEntityType == 0x26
                || model.mEntityType == 0x27
                )
            {
                if (nVertices > 0)
                    nVertices -= 1;
            }

            for (unsigned v = 0; v <= nVertices; v++)
            {
                if (nVertices == 0) continue;
                model.mVertices.emplace_back(fb.LoadVector<std::int16_t, 3>());
            }

            if (nVertices == 0)
            {
                // 2D objects
                if ((model.mEntityFlags & EF_UNBOUNDED) 
                    && (model.mEntityFlags & EF_2D_OBJECT) && (nComponents == 1))
                {
                    fb.Skip(2);
                    model.mSprite = fb.GetUint16LE();
                    fb.Skip(4);
                }
                else
                {
                    model.mSprite = -1;
                }
            }
            else
            {
                // nComponents - 1 ? This doesn't seem right... something a little wrong with house?
                if (model.mEntityType == 0xa) nComponents -= 1;
                for (unsigned int component = 0; component < nComponents; component++)
                {
                    std::vector<std::uint16_t> nFaces{};
                    // There can be multiple "components" 
                    while (fb.GetUint16LE() == 0)
                    {
                        nFaces.emplace_back(fb.GetUint16LE());
                        fb.Skip(4); // Offset?
                    }
                    fb.Skip(-2); // Go back 
                    if (nFaces.empty())
                    {
                        continue;
                    }

                    assert(!nFaces.empty());

                    for (const auto faces : nFaces)
                    {
                        for (unsigned k = 0; k < faces; k++)
                        {
                            model.mPalettes.push_back(fb.GetUint8());
                            // There are four colours defined face color, edge color, ??, and ??
                            for (unsigned c = 0; c < 4; c++)
                            {
                                if (c == 0)
                                {
                                    auto color = fb.GetUint8();
                                    model.mFaceColors.push_back(color); 
                                }
                                // for now we ignore the edge colors and other colors
                                else
                                {
                                    auto color = fb.GetUint8();
                                }
                            }
                            fb.Skip(3); // Offset?
                        }
                        for (unsigned k = 0; k < faces; k++)
                        {
                            unsigned vertI;
                            std::vector<std::uint16_t> vertIndices;
                            while ((vertI = fb.GetUint8()) != 0xff)
                            {
                                // If we are at or beyond the component where the 
                                // vertices change over then we need to add the respective offset
                                if (component >= changeOverComponent)
                                    vertI += changeOverVerticesOffset;
                                vertIndices.emplace_back(vertI);
                            }
                            model.mFaces.emplace_back(vertIndices);
                        }
                    }
                }
            }
        }

        models.emplace_back(std::move(model));
    }

    return models;
}

std::vector<Model> LoadTBL(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto mapbuf = fb.Find(TAG_MAP);
    auto appbuf = fb.Find(TAG_APP);
    auto gidbuf = fb.Find(TAG_GID);
    auto datbuf = fb.Find(TAG_DAT);

    const auto names = LoadModelNames(mapbuf);
    return LoadModels(datbuf, names);
}

}
