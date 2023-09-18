#pragma once

#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Files.hpp>
#include <TLib/Containers/UnorderedMap.hpp>

struct TextureManager
{
    UnorderedMap<String, Texture> textures;

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
                rendlog->error("Failed to load texture: {}", pathStr);
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