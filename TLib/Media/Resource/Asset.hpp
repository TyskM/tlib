#pragma once
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Files.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Containers/Array.hpp>
#include <functional>

template <typename T>
struct Asset
{
private:
    T* res = nullptr;

public:
    Asset(T* t) { res =  t; }
    Asset(T& t) { res = &t; }
    Asset() = default;

    T& get() { return *res; }
    T& get() const { return *res; }

    bool valid() const
    { return res != nullptr; }

    explicit operator bool() const
    { return valid(); }

    T* operator->() { return res; }
    T* operator->() const { return res; }

    bool operator==(const Asset<T>& other) const { return res == other.res; }
    bool operator!=(const Asset<T>& other) const { return res != other.res; }
};

template <typename T>
struct AssetCacheNoDelete
{
private:
    using LoadFunc = std::function<void(T&, const fs::path& path)>;
    using pair = Pair<Path, UPtr<T>>;
    Vector<pair> cache;
    LoadFunc loadFunc;

public:
    AssetCacheNoDelete(const LoadFunc& loadFunc)
    {
        this->loadFunc = loadFunc;
    }

    Asset<T> getOrLoad(const fs::path& path)
    {
        if (!fs::exists(path))
        { tlog::info("'{}' doesn't exist!", path.string()); }
        else if (!fs::is_regular_file(path))
        { tlog::info("'{}' isn't a file.", path.string()); }

        for (auto& [k, v] : cache)
        {
            if (fs::equivalent(k, path))
            { return Asset<T>(*v); }
        }

        tlog::info("Loading {}", path.string());
        cache.emplace_back();
        auto& back = cache.back();
        back.first = path;
        back.second = makeUnique<T>();
        loadFunc(*back.second, path);
        return Asset<T>(*back.second);
    }

    Path getPath(const Asset<T>& asset) const
    {
        for (const pair& kvp : cache)
        {
            if (kvp.second.get() == &asset.get())
            { return kvp.first; }
        }
    }
};


