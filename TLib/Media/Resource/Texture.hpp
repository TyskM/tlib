#pragma once

#define NOMINMAX

#include <TLib/DataStructures.hpp>
#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Files.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Macros.hpp>
#include <TLib/String.hpp>
#include <TLib/thirdparty/stbi.hpp>

#include "IResource.hpp"
#include "TextureData.hpp"

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

enum class UVMode
{
    Unknown             [[maybe_unused]] = -1,
    Repeat              [[maybe_unused]] = GL_REPEAT,
    MirroredRepeat      [[maybe_unused]] = GL_MIRRORED_REPEAT,
    ClampToEdge         [[maybe_unused]] = GL_CLAMP_TO_EDGE,
    MirroredClampToEdge [[maybe_unused]] = GL_MIRROR_CLAMP_TO_EDGE,
    ClampToBorder       [[maybe_unused]] = GL_CLAMP_TO_BORDER
};

// OpenGL Texture
struct Texture : NonCopyable
{
private:
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    static constexpr TextureFiltering   defaultTexFiltering   = TextureFiltering::Linear;
    static constexpr TexPixelFormats    defaultFormat         = TexPixelFormats::RGBA;
    static constexpr TexInternalFormats defaultInternalFormat = TexInternalFormats::RGBA;
    static constexpr UVMode             defaultUVMode         = UVMode::ClampToEdge;
    static inline GLubyte fallbackImage[2][2][4] =
    {
        { {255, 0, 255, 255}, {0,   0, 0,   255} },
        { {0,   0, 0,   255}, {255, 0, 255, 255} }
    };

    GLuint glHandle        =  0;
    int    width           =  0;
    int    height          =  0;
    int    boundSlot       = -1;
    fs::path _path;

public:
    Texture() { }
    Texture(const String& filePath)
    { loadFromFile(filePath); }

    Texture(Texture&& other) noexcept
    {
        glHandle = other.glHandle;
        other.glHandle  = 0;
        other.boundSlot = -1;

        width      = other.width;
        height     = other.height;
        boundSlot  = other.boundSlot;
        _path      = other._path;
    }

    Texture& operator=(Texture&& other) noexcept
    {
        glHandle = other.glHandle;
        other.glHandle  = 0;
        other.boundSlot = -1;

        width      = other.width;
        height     = other.height;
        boundSlot  = other.boundSlot;
        _path      = other._path;
    }

    ~Texture() { reset(); }

    void reset()
    {
        if (created())
        {
            GL_CHECK(glDeleteTextures(1, &glHandle));
            glHandle        =  0;
            width           =  0;
            height          =  0;
            boundSlot       = -1;
        }
    }

    void create()
    {
        reset();
        GL_CHECK(glGenTextures(1, &glHandle));
    }

    bool loadFromFile(const fs::path& path)
    {
        if (!created()) { create(); }

        TextureData data(path, 4);

        if (!data.valid()) // Error!!
        {
            tlog::error("Failed to load image from path ('{}')", path.string());

            if (stbi_failure_reason())
            { tlog::error("\tReason: {}", stbi_failure_reason()); }

            tlog::error("\tWorking directory: {}", fs::current_path().string());

            loadFallbackTexture();
            return false;
        }
        else
        {
            _path = path;

            // TODO: Support different formats?
            // https://stackoverflow.com/questions/23150123/loading-png-with-stb-image-for-opengl-texture-gives-wrong-colors
            setData(data, defaultFormat, defaultInternalFormat);
        }

        return true;
    }

    void setData(const TextureData&       data,
                 const TexPixelFormats    format         = defaultFormat,
                 const TexInternalFormats internalFormat = defaultInternalFormat,
                 const bool               generateMipmap = true)
    {
        setData(data.ptr(), data.width(), data.height(), defaultFormat, defaultInternalFormat);
    }

    void setData(
        const void*              data,
        const int                width,
        const int                height,
        const TexPixelFormats    format         = defaultFormat,
        const TexInternalFormats internalFormat = defaultInternalFormat,
        const bool               generateMipmap = true)
    {
        if (!created()) { create(); }

        this->width  = width;
        this->height = height;

        setFilter(defaultTexFiltering);
        setUVMode(defaultUVMode);
        
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

    [[nodiscard]]
    bool created() const
    { return glHandle != 0 && SDL_GL_GetCurrentContext() != NULL; }

    [[nodiscard]]
    GLuint handle() const
    { return glHandle; }

    [[nodiscard]]
    Vector2i getSize() const
    { return Vector2i{ width, height }; }

    // Returns the path passed during loadFromFile()
    // If the texture is empty or the data was passed in without loadFromFile
    // then path().empty() will return true.
    fs::path path() const { return _path; }

    void setPath(const fs::path& apath) { _path = apath; }

    bool isFromSamePath(const fs::path& otherPath) const
    {
        // If one or both paths are empty, this should probably return false.
        if (path().empty())
        {
            #if TLIB_DEBUG
            tlog::debug("Tried to compare paths where one or both was empty!\n\tPath 1: {}\n\tPath 2: {}",
                path().string(), otherPath.string());
            #endif
            return false;
        }

        return fs::equivalent(path(), otherPath);
    }

    bool isFromSamePath(const Texture& otherTex) const
    { return isFromSamePath(otherTex.path()); }

    // TODO: is unpack alignment per texture?? where am i>:>??????
    void setUnpackAlignment(int value)
    {
        bind();
        glPixelStorei(GL_UNPACK_ALIGNMENT, value);
    }

    // Equivalent to setting GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T
    void setUVMode(UVMode u, UVMode v)
    {
        bind();
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(u)) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(v)) );
    }

    void setUVMode(UVMode uv)
    { setUVMode(uv, uv); }

    void setFilter(TextureFiltering min, TextureFiltering max)
    {
        bind();
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(min)) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(max)) );
    }

    void setFilter(TextureFiltering mode)
    { setFilter(mode, mode); }

    // BGRA Mask example
    // int32_t swizzleMask[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
    void setSwizzle(const int32_t* mask)
    {
        bind();
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, mask);
    }

    void loadFallbackTexture()
    {
        setData(fallbackImage, 2, 2);
        setFilter(TextureFiltering::Nearest);
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