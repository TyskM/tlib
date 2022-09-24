#pragma once

#include "Texture.hpp"
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct TextureCache
{
    std::unordered_map<std::string, Texture> textures;

    void clear() { textures.clear(); }

    void load(const fs::path& path)
    {
        const auto& normalizedPath = path.lexically_normal();
        const auto& pathStr = normalizedPath.string();
        if (!textures.contains(pathStr))
        {
            std::cout << "Loading new texture: " << pathStr << std::endl;
            textures.try_emplace(pathStr);
            textures[pathStr].loadFromFile(pathStr, TextureFiltering::Nearest);
        }
    }

    Texture& getTex(const fs::path& path)
    {
        load(path);
        return textures[path.string()];
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