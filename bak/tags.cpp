#include "bak/tags.hpp"

#include "bak/file/fileBuffer.hpp"

#include "com/logger.hpp"

namespace BAK {

void Tags::Load(FileBuffer& fb)
{
    unsigned int n = fb.GetUint16LE();
    for (unsigned int i = 0; i < n; i++)
    {
        unsigned int id = fb.GetUint16LE();
        std::string name = fb.GetString();
        mTags.emplace(id, name);
    }
}

std::optional<std::string> Tags::GetTag(Tag tag) const
{
    const auto it = mTags.find(tag);
    if (it != mTags.end())
    {
        return std::make_optional(it->second);
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<Tag > Tags::FindTag(const std::string& tag) const
{
    const auto it = std::find_if(mTags.begin(), mTags.end(),
        [&](const auto& it)
        {
            return it.second == tag;
        });

    if (it != mTags.end())
    {
        return std::make_optional(it->first);
    }
    else
    {
        return std::nullopt;
    }
}

void Tags::DumpTags() const
{
    if (mTags.size() == 0)
    {
        Logging::LogDebug(__FUNCTION__) << "No Tags\n";
    }
    for (const auto& [key, tag] : mTags)
    {
        Logging::LogDebug(__FUNCTION__) << " Key - " << key << " tag: " << tag << "\n";
    }
}

}
