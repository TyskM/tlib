
#pragma once

#define NOMINMAX

#include <TLib/String.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/thirdparty/stbi.hpp>

// CPU Texture information
class TextureData : NonAssignable
{
    stbi_uc* _ptr          = nullptr;
    int      _width        = 0;
    int      _height       = 0;
    int      _channelCount = 0;

public:
    TextureData() = default;

    // @param reqComp: the required number of channels. ex: 4 for RGBA
    TextureData(const fs::path& path, int reqComp = 4)
    { loadFromPath(path, reqComp); }

    TextureData(stbi_uc* data, int width, int height, int channelCount) :
        _ptr{data}, _width{_width}, _height{_height}, _channelCount{_channelCount} { }

    // TODO: should be names loadFromFile
    bool loadFromPath(const Path& path, int reqComp = 4)
    {
        stbi_set_flip_vertically_on_load(true);

        bool pathExists = fs::exists(path);
        if (!pathExists)
        {
            tlog::error("TextureData::loadFromFile (FILE DOES NOT EXIST): Failed to load file from path '{}'", path.string());
            return false;
        }

        bool pathIsFile = fs::is_regular_file(path);
        if (!pathIsFile)
        {
            tlog::error("TextureData::loadFromFile (PATH IS NOT FILE): Failed to load file from path '{}'", path.string());
            return false;
        }

        stbi_uc* tempptr = stbi_load(path.string().c_str(), &_width, &_height, &_channelCount, reqComp);
        if (tempptr == NULL)
        {
            tlog::error("TextureData::loadFromFile (INVALID DATA/UNKNOWN): Failed to load file from path '{}'", path.string());
            return false;
        }

        reset();
        _ptr = tempptr;
        return true;
    }

    bool loadFromMemory(uint8_t const* data, int32_t length, int reqComp = 4)
    {
        stbi_uc* tempptr = stbi_load_from_memory(data, length, &_width, &_height, &_channelCount, reqComp);

        if (tempptr == NULL)
        {
            tlog::error("TextureData::loadFromMemory : Failed to load texture from memory with length {} ({})", length, stbi_failure_reason());
            return false;
        }

        reset();
        _ptr = tempptr;
        return true;
    }

    void loadFromMemoryRaw(uint8_t const* rawData, int width, int height, int channels)
    {
        reset();

        int len = width * height * channels;
        _ptr = (stbi_uc*)malloc(len);
        memcpy(_ptr, rawData, len);
        ASSERT(_ptr);

        this->_width        = width;
        this->_height       = height;
        this->_channelCount = channels;
    }

    void reset()
    { stbi_image_free(_ptr); }

    bool valid() const { return _ptr != nullptr; }

    inline Vector2i size()          const { return Vector2i(_width, _height); }
    inline int      width()         const { return _width; }
    inline int      height()        const { return _height; }
    inline int      channelCount()  const { return _channelCount; }
    inline stbi_uc* ptr()           const { return _ptr; }

    ~TextureData() noexcept { reset(); }

    operator bool() { valid(); }
};
