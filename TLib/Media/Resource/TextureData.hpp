
#pragma once

#define NOMINMAX

#include <TLib/String.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/thirdparty/stbi.hpp>

// CPU Texture information
struct TextureData : NonCopyable
{
    stbi_uc* ptr          = nullptr;
    int      width        = 0;
    int      height       = 0;
    int      channelCount = 0;

    bool loadFromPath(const String& path, int reqComp = 4)
    {
        reset();
        auto tempptr = stbi_load(path.c_str(), &width, &height, &channelCount, reqComp);

        if (stbi_failure_reason())
        {
            tlog::error("TextureData: Failed to load file from path '{}'", path);
            ptr = nullptr;
            return false;
        }

        ptr = tempptr;
        return true;
    }

    void reset()
    { stbi_image_free(ptr); }

    bool valid() const { return ptr != nullptr; }

    TextureData() = default;

    // @param reqComp: the required number of channels. ex: 4 for RGBA
    TextureData(const String& path, int reqComp = 4)
    { loadFromPath(path, reqComp); }

    TextureData(stbi_uc* data, int width, int height, int channelCount) :
        ptr{ data }, width{ width }, height{ height }, channelCount{ channelCount } { }

    ~TextureData() noexcept { reset(); }

    TextureData(TextureData&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    };

    operator bool() { valid(); }
};
