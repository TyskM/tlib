#pragma once

#include "Texture.hpp"
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct TextureManager
{
    std::unordered_map<std::string, Texture> textures;

    void clear() { textures.clear(); }

    Texture& getOrLoad(const fs::path& path)
    {
        const auto& normalizedPath = path.lexically_normal();
        const auto& pathStr = normalizedPath.string();
        if (!textures.contains(pathStr))
        {
            std::cout << "Loading new texture: " << pathStr << std::endl;
            textures[pathStr].loadFromFile(pathStr, TextureFiltering::Nearest);
        }
        return textures.at(pathStr);
    }

    std::string getTexPath(Texture* tex)
    {
        for (auto& [str, tex2] : textures)
        {
            if (&tex2 == tex) { return str; }
        }
        std::cerr << "!!! ERROR: Could not find path for a texture\n";
        return "UNDEFINED";
    }
};