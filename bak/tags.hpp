#pragma once

#include "com/strongType.hpp"

#include "bak/fileBufferFactory.hpp"

#include <optional>
#include <unordered_map>

namespace BAK {

using Tag = StrongType<unsigned, struct TagT>;

class Tags
{
public:
    void Load(FileBuffer& fb);
    std::optional<std::string> GetTag(Tag) const;
    std::optional<Tag> FindTag(const std::string&) const;
private:
    std::unordered_map<Tag, std::string> mTags;
};

}
