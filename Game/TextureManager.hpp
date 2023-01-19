#pragma once

#include "../MediaGL/Texture.hpp"
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct TextureManager
{
    std::unordered_map<std::string, Texture> textures;

    void clear() { textures.clear(); }

    template <typename... Args>
    Texture* getOrLoad(const fs::path& path, Args... args)
    {
        const auto& normalizedPath = path.lexically_normal();
        const auto& pathStr = normalizedPath.string();
        if (!textures.contains(pathStr))
        {
            //std::cout << "Loading new texture: " << pathStr << std::endl;
            if (!textures[pathStr].loadFromFile(pathStr, args...))
            {
                textures.erase(pathStr);
                return nullptr;
            }
        }
        return &textures.at(pathStr);
    }

    std::string getTexPath(Texture* tex)
    {
        for (auto& [str, tex2] : textures)
        {
            if (&tex2 == tex) { return str; }
        }
        tlog::error("Could not find path for a texture");
        return "UNDEFINED";
    }
};