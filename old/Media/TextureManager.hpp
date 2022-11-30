#pragma once

#include "Renderer.hpp"
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

// Require C++20
struct TextureManager
{
    std::unordered_map<std::string, Texture> map;
    Renderer* renderer = nullptr;

    TextureManager(Renderer& renderer) : renderer{&renderer}
    { }

    void clear()
    {
        map.clear();
    }

    void load(const fs::path& path)
    {
        const auto& normalizedPath = path.lexically_normal();
        const auto& pathStr = normalizedPath.string();
        if (!map.contains(pathStr))
        {
            std::cout << "Loading new texture: " << pathStr << std::endl;
            map.try_emplace(pathStr);
            map[pathStr].loadFromPath(*renderer, pathStr);
        }
    }

    Texture& get(const fs::path& path)
    {
        return map[path.string()];
    }

    Texture& getOrLoad(const fs::path& path)
    {
        load(path);
        return map[path.string()];
    }

    std::string getTexPath(Texture* tex)
    {
        for (auto& [str, tex2] : map)
        {
            if (&tex2 == tex) { return str; }
        }
        std::cerr << "!!! ERROR: Could not find path for a texture\n";
        return "UNDEFINED";
    }
};