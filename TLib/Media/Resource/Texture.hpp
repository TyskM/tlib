#pragma once

#define NOMINMAX
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <TLib/DataStructures.hpp>
#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Files.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Macros.hpp>
#include <TLib/String.hpp>

enum class TextureFiltering
{
    Unknown [[maybe_unused]] = -1,
    Nearest [[maybe_unused]] = GL_NEAREST,
    Linear  [[maybe_unused]] = GL_LINEAR
};

// TODO: add the rest of the formats
enum class TexInternalFormats : int
{
    Unknown         [[maybe_unused]] = -1,
    DEPTH_COMPONENT [[maybe_unused]] = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   [[maybe_unused]] = GL_DEPTH_STENCIL,
    RED             [[maybe_unused]] = GL_RED,
    RG              [[maybe_unused]] = GL_RG,
    RGB             [[maybe_unused]] = GL_RGB,
    RGBA            [[maybe_unused]] = GL_RGBA
};

enum class TexPixelFormats : int
{
    Unknown         [[maybe_unused]] = -1,
    RED             [[maybe_unused]] = GL_RED,
    RG              [[maybe_unused]] = GL_RG,
    RGB             [[maybe_unused]] = GL_RGB,
    BGR             [[maybe_unused]] = GL_BGR,
    RGBA            [[maybe_unused]] = GL_RGBA,
    BGRA            [[maybe_unused]] = GL_BGRA,
    RED_INTEGER     [[maybe_unused]] = GL_RED_INTEGER,
    RG_INTEGER      [[maybe_unused]] = GL_RG_INTEGER,
    RGB_INTEGER     [[maybe_unused]] = GL_RGB_INTEGER,
    BGR_INTEGER     [[maybe_unused]] = GL_BGR_INTEGER,
    RGBA_INTEGER    [[maybe_unused]] = GL_RGBA_INTEGER,
    BGRA_INTEGER    [[maybe_unused]] = GL_BGRA_INTEGER,
    STENCIL_INDEX   [[maybe_unused]] = GL_STENCIL_INDEX,
    DEPTH_COMPONENT [[maybe_unused]] = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   [[maybe_unused]] = GL_DEPTH_STENCIL
};

struct TextureData : NonCopyable
{
    stbi_uc* ptr          = nullptr;
    int      width        = 0;
    int      height       = 0;
    int      channelCount = 0;

    void loadFromPath(const String& path, int reqComp = 4)
    {
        reset();
        ptr = stbi_load(path.c_str(), &width, &height, &channelCount, reqComp);
    }

    void reset()
    { stbi_image_free(ptr); }

    bool valid() { return ptr != nullptr; }

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

struct Texture : NonAssignable
{
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    static inline TextureFiltering   defaultTexFiltering   = TextureFiltering::Linear;
    static inline TexPixelFormats    defaultFormat         = TexPixelFormats::RGBA;
    static inline TexInternalFormats defaultInternalFormat = TexInternalFormats::RGBA;
    static inline GLubyte fallbackImage[2][2][4] =
    {
        { {255, 0, 255, 255}, {0,   0, 0,   255} },
        { {0,   0, 0,   255}, {255, 0, 255, 255} }
    };

    GLuint             glHandle        =  0;
    int                width           =  0;
    int                height          =  0;
    int                boundSlot       = -1;
    int                unpackAlignment =  4;
    TextureFiltering   filtering       = TextureFiltering::Unknown;
    TexPixelFormats    format          = TexPixelFormats::Unknown;
    TexInternalFormats internalFormat  = TexInternalFormats::Unknown;

    Texture() { }
    Texture(const std::string& filePath, TextureFiltering texFiltering = defaultTexFiltering)
    { loadFromFile(filePath, texFiltering); }

    ~Texture() { reset(); }

    void reset()
    {
        if (created())
        {
            GL_CHECK(glDeleteTextures(1, &glHandle));
            glHandle        = 0;
            width           = 0;
            height          = 0;
            unpackAlignment = 4;
            boundSlot       = -1;
            filtering       = TextureFiltering::Unknown;
            format          = TexPixelFormats::Unknown;
            internalFormat  = TexInternalFormats::Unknown;
        }
    }

    void create()
    {
        reset();
        GL_CHECK(glGenTextures(1, &glHandle));
    }

    bool loadFromFile(const String& filePath, TextureFiltering texFiltering = defaultTexFiltering)
    {
        if (!created()) { create(); }

        TextureData data(filePath, 4);

        if (!data.valid()) // Error!!
        {
            tlog::error("Failed to load image from path ('{}')", filePath);

            if (stbi_failure_reason())
            { tlog::error("\tReason: {}", stbi_failure_reason()); }

            tlog::error("\tWorking directory: {}", fs::current_path().string());

            loadFallbackTexture();
            return false;
        }
        else
        {
            // TODO: Support different formats?
            // https://stackoverflow.com/questions/23150123/loading-png-with-stb-image-for-opengl-texture-gives-wrong-colors
            setData(data, texFiltering, defaultFormat, defaultInternalFormat);
        }

        return true;
    }

    void setData(const TextureData& data,
                 TextureFiltering texFiltering = defaultTexFiltering,
                 TexPixelFormats format = defaultFormat,
                 TexInternalFormats internalFormat = defaultInternalFormat, bool generateMipmap = true)
    {
        setData(data.ptr, data.width, data.height, texFiltering, defaultFormat, defaultInternalFormat);
    }

    void setData(
        const void* data, int width, int height,
        TextureFiltering texFiltering = defaultTexFiltering,
        TexPixelFormats format = defaultFormat,
        TexInternalFormats internalFormat = defaultInternalFormat, bool generateMipmap = true)
    {
        if (!created()) { create(); }

        this->width          = width;
        this->height         = height;
        this->filtering      = texFiltering;
        this->format         = format;
        this->internalFormat = internalFormat;

        bind();
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(texFiltering)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(texFiltering)));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)format, GL_UNSIGNED_BYTE, data));
        if (generateMipmap)
        { GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D)); }
    }

    void bind(int slot = 0)
    {
        ASSERT(created());

        if (slot > glState.boundTextures.size())
        { glState.boundTextures.resize(slot, nullptr); }
        else if (glState.boundTextures[slot] == this) { return; }

        GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, glHandle));
        glState.boundTextures[slot] = this;
    }

    static inline void unbind(int slot = 0)
    {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
        glState.boundTextures[slot] = nullptr;
    }

    bool created() const
    { return glHandle != 0 && SDL_GL_GetCurrentContext() != NULL; }

    Vector2i getSize() const
    { return Vector2i{ width, height }; }

    void setUnpackAlignment(int value)
    {
        if (unpackAlignment != value)
        {
            unpackAlignment = value;
            bind();
            glPixelStorei(GL_UNPACK_ALIGNMENT, value);
        }
    }

    // Private
    void loadFallbackTexture()
    {
        setData(fallbackImage, 2, 2, TextureFiltering::Nearest);
    }
};

// SubTexture to make using parts of a texture atlas more convenient.
// Can be passed to Renderers drawTexture() method.
// Becomes invalid after resizing a texture.
struct SubTexture
{
    Texture* texture = nullptr;
    Recti rect;

    SubTexture() { }
    SubTexture(Texture& tex, const Recti& rect) : texture{ &tex }, rect{ rect } { }
    SubTexture(Texture& tex) : texture{ &tex }, rect{ Vector2i{0,0}, texture->getSize() } { }
};