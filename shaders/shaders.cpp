#include "shaders/shaders.hpp"

#include <array>
#include <algorithm>

namespace Shaders {

static constexpr auto ShaderFiles = std::to_array<std::pair<const char*, const char*>>({
#include "shaders/shader_binary_cache.hxx"
});

std::optional<std::string> GetFile(std::string shaderFile)
{
    auto it = std::find_if(ShaderFiles.begin(), ShaderFiles.end(),
        [&shaderFile](const auto& shader)
        {
            return shaderFile == std::string(shader.first);
        });

    if (it != ShaderFiles.end())
    {
        return it->second;
    }

    return std::nullopt;
}
}
