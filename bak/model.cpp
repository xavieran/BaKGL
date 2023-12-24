#include "bak/dataTags.hpp"
#include "bak/model.hpp"

#include "com/ostream.hpp"
#include "graphics/glm.hpp"

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

struct EdgeX
{
    std::uint8_t mPalette;
    glm::uvec4 mColor;
    std::uint8_t mGroup;
    std::uint16_t mVertexOffset;
    std::vector<std::uint16_t> mVertexIndices;
};

struct FaceX
{
    unsigned mType;
    unsigned mEdgeCount;
    unsigned mEdgeOffset;
    std::vector<EdgeX> mEdges{};
};

struct MeshX
{
    unsigned mVertexCount;
    unsigned mVertexOffset;
    unsigned mVertexIndexTransform;
    unsigned mFaceCount;
    unsigned mFaceOffset;
    std::vector<glm::vec3> mVertices;
    std::vector<FaceX> mFaces;
};

struct ModelX
{
    unsigned mMeshCount;
    unsigned mMeshOffset;

    std::vector<MeshX> mMeshs;
};

struct Animation
{
    std::array<char, 7> mData;
};

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
        auto offset = offsets[i];
        model.mName = itemNames[i];
        fb.Dump(16);
        model.mEntityFlags = fb.GetUint8();
        model.mEntityType = fb.GetUint8();
        model.mTerrainType = fb.GetUint8();
        model.mScale = fb.GetUint8();

        auto animCount = fb.GetUint16LE();
        auto animOffset = fb.GetUint16LE();
        // fb.Skip(4); 
        logger.Info() << model.mName << " @" << offsets[i] << " ( " << offset << " ) " << "\n";
        logger.Info() << "Anim: " << animCount << " " << animOffset << "\n";
        if (model.mName == "boom")
        {
            // to keep the indices aligned
            models.emplace_back(models.back());
            continue;
        }

        auto modelCount = fb.GetUint16LE();
        auto baseOffset = fb.GetUint16LE();

        auto u1 = fb.GetUint8();
        auto u2 = fb.GetUint8();
        logger.Info() << "MC: " << modelCount<< " " << baseOffset<< " (" << std::hex << +u1 << " " << +u2 << std::dec << ")\n";

        std::vector<ModelX> modelXs{};

        bool bounded = !(model.mEntityFlags & EF_UNBOUNDED);
        if (bounded)
        {
            fb.Dump(12);
            model.mMin = fb.LoadVector<std::int16_t, 3>();
            model.mMax = fb.LoadVector<std::int16_t, 3>();
        }

        for (auto modelI = 0; modelI < modelCount; modelI++)
        {
            fb.Dump(6);
            auto u_1_1 = fb.GetUint8();
            auto u_1_2 = fb.GetUint8();

            unsigned meshCount = fb.GetUint16LE();
            unsigned meshOffset = fb.GetUint16LE();
            modelXs.emplace_back(ModelX{meshCount, meshOffset, {}});
            logger.Info() << " Model U (" << +u_1_1 << ", " << +u_1_2 << ") mesh cnt " << meshCount << " meshOff: " << meshOffset << "\n";
        }

        offset += 14 + (bounded ? 12 : 0);
        logger.Info() << "Offset now: " << offset << "\n";

        // this doesn't work
        //if (animCount > 0)
        //{
        //    
        //    fb.Seek(offset + animOffset - baseOffset);
        //    for (auto animI = 0; animI < animCount; animI++)
        //    {
        //        fb.DumpAndSkip(7);
        //    }
        //}

        std::vector<std::pair<unsigned, unsigned>> vertexSets{};
        std::vector<unsigned> vertexSums{};
        for (auto modelI = 0; modelI < modelCount; modelI++)
        {

            auto& modelX = modelXs[modelI];
            fb.Seek(offset + modelX.mMeshOffset - baseOffset);
            for (unsigned meshI = 0; meshI < modelX.mMeshCount; meshI++)
            {
                logger.Info() << "Mesh #" << meshI << "\n";
                fb.Dump(14);
                fb.Skip(3);
                unsigned vertices = fb.GetUint8();
                unsigned vertexOffset = fb.GetUint16LE();
                unsigned faceCount = fb.GetUint16LE();
                unsigned faceOffset = fb.GetUint16LE();
                if (vertexSets.empty())
                {
                    vertexSets.emplace_back(vertices, vertexOffset);
                    vertexSums.emplace_back(0);
                }
                else
                {
                    if (std::make_pair(vertices, vertexOffset) != vertexSets.back())
                    {
                        vertexSums.emplace_back(vertexSums.back() + vertexSets.back().first);
                        vertexSets.emplace_back(vertices, vertexOffset);
                    }
                }

                modelX.mMeshs.emplace_back(MeshX{vertices, vertexOffset, vertexSums.back(), faceCount, faceOffset, {}, {}});
                logger.Info() << " VC: " << vertices << " VO: " << vertexOffset << " FC: " << faceCount << " FO: " << faceOffset << "\n";
                fb.Skip(4);
            }
        }

        std::vector<glm::i32vec3> vertices{};
        unsigned vs = 0;
        logger.Info() << "Vertices start at: " << fb.Tell() << "\n";
        fb.Dump(6);
        for (auto [vertexCount, vertexOffset] : vertexSets)
        {
            logger.Info() << "VC: " << vertexCount << " " << vertexOffset << "\n";
            logger.Info() << "ULocation: " << fb.Tell() << "\n";
            logger.Info() << "ULocation calc: " << offset - baseOffset + vertexOffset << "\n";
            fb.Seek(offset - baseOffset + vertexOffset);
            for (unsigned vv = 0; vv < vertexCount; vv++)
            {
                vertices.emplace_back(fb.LoadVector<std::int16_t, 3>());
            }
        }
        logger.Info() << "Vertices end at " << fb.Tell() << "\n";
        fb.Dump(8);
        model.mVertices = vertices;

        for (unsigned modelI = 0; modelI < modelCount; modelI++)
        {
            auto& modelX = modelXs[modelI];
            for (unsigned meshI = 0; meshI < modelX.mMeshCount; meshI++)
            {
                auto& meshX = modelX.mMeshs[meshI];
                auto fo = offset - baseOffset + meshX.mFaceOffset;
                logger.Info() << meshI << " Off: " << offset << " BO: " << baseOffset << " animO: " << animOffset << " FO: " << meshX.mFaceOffset << std::endl;
                logger.Info() << meshI << " -- Sook from: " << fb.Tell() << " to: " << fo << "\n";
                fb.Seek(fo);
                fb.Dump(8);
                for (unsigned faceI = 0; faceI < meshX.mFaceCount; faceI++)
                {
                    auto faceType = fb.GetUint16LE();
                    auto edgeCount = fb.GetUint16LE();
                    auto edgeOffset = fb.GetUint16LE();
                    fb.Skip(2);
                    auto& faceX = meshX.mFaces.emplace_back(FaceX{faceType, edgeCount, edgeOffset});
                    logger.Info() << "Mesh Num: " << meshI << "\n";
                    logger.Info() << "Face # " << faceI << "\n";
                    logger.Info() << "tp " << faceType << " ec " << edgeCount << " eo " << edgeOffset << "\n";

                    if (faceType == 2)
                    {
                        model.mSprite = edgeCount;
                        continue;
                    }
                }

                for (unsigned faceI = 0; faceI < meshX.mFaceCount; faceI++)
                {
                    auto& faceX = meshX.mFaces[faceI];
                    if (faceX.mType == 2) continue;
                    fb.Seek(offset - baseOffset + faceX.mEdgeOffset);
                    unsigned edgeSeekOffset = offset - baseOffset + faceX.mEdgeOffset;
                    for (unsigned edgeI = 0; edgeI < faceX.mEdgeCount; edgeI++)
                    {
                        fb.Seek(edgeSeekOffset);
                        logger.Info() << + edgeI << " -- Sook to: " << edgeSeekOffset << " sz: " << fb.GetSize() << "\n";
                        fb.Dump(8);
                        auto palette = fb.GetUint8();
                        model.mPalettes.emplace_back(palette);
                        auto color = fb.LoadVector<std::uint8_t, 4>();
                        model.mFaceColors.emplace_back(color.x);
                        auto group = fb.GetUint8();
                        auto vertexOffset = fb.GetUint16LE();
                        auto& edgeX = faceX.mEdges.emplace_back(EdgeX{palette, color, group, vertexOffset, {}});
                        edgeSeekOffset = fb.Tell();
                        logger.Info() << "Edge " << edgeI << " " << +edgeX.mPalette << " color " << edgeX.mColor << " group " << +edgeX.mGroup << " vo " << edgeX.mVertexOffset << "\n";

                        logger.Info() << "Going to vertex offset: " << offset - baseOffset + vertexOffset << "\n";
                        fb.Seek(offset - baseOffset + vertexOffset);
                        std::uint8_t vertexIndex;
                        std::stringstream ss{};
                        while ((vertexIndex = fb.GetUint8()) != 0xff)
                        {
                            ss << +vertexIndex << " ";
                            edgeX.mVertexIndices.emplace_back(static_cast<std::uint16_t>(vertexIndex) + meshX.mVertexIndexTransform);
                        }
                        model.mFaces.emplace_back(edgeX.mVertexIndices);
                        ss << vertexIndex << "\n";
                        logger.Info() << ss.str() << "\n";
                    }
                }
            }
        }

        //if (nVertices == 0)
        //{
        //    // 2D objects
        //    if ((model.mEntityFlags & EF_UNBOUNDED) 
        //        && (model.mEntityFlags & EF_2D_OBJECT) && (meshCount == 1))
        //    {
        //        fb.Skip(2);
        //        model.mSprite = fb.GetUint16LE();
        //        fb.Skip(4);
        //    }
        //    else
        //    {
        //        model.mSprite = -1;
        //    }
        //}

        models.emplace_back(std::move(model));
    }

    return models;
}

std::vector<Model> LoadTBL(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    auto mapbuf = fb.Find(DataTag::MAP);
    auto appbuf = fb.Find(DataTag::APP);
    auto gidbuf = fb.Find(DataTag::GID);
    auto datbuf = fb.Find(DataTag::DAT);

    const auto names = LoadModelNames(mapbuf);
    std::stringstream ss{};
    for (const auto& name : names)
    {
        ss << " " << name << ",";
    }
    logger.Info() << "Loading models: " << ss.str() << "\n";
    return LoadModels(datbuf, names);
}

}
