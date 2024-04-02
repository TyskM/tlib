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

enum class TextureMinFilter : GLenum
{
    Nearest              = GL_NEAREST,
    Linear               = GL_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    LinearMipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipmapLinear  = GL_NEAREST_MIPMAP_LINEAR,
    LinearMipmapLinear   = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter : GLenum
{
    Nearest = GL_NEAREST,
    Linear  = GL_LINEAR
};

// TODO: Deprecated
enum class TextureFiltering
{
    Nearest = GL_NEAREST,
    Linear  = GL_LINEAR
};

// TODO: add the rest of the formats
enum class TexInternalFormats : int
{
    Unknown         [[maybe_unused]] = -1,

    // Base Formats
    DEPTH_COMPONENT [[maybe_unused]] = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   [[maybe_unused]] = GL_DEPTH_STENCIL,
    RED             [[maybe_unused]] = GL_RED,
    RG              [[maybe_unused]] = GL_RG,
    RGB             [[maybe_unused]] = GL_RGB,
    RGBA            [[maybe_unused]] = GL_RGBA,

    // Sized Formats
    RGBA32f = GL_RGBA32F,
    RGBA16f = GL_RGBA16F,
    RGBA8   = GL_RGBA8
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

static inline int32_t getFormatSize(TexInternalFormats format)
{
    switch (format)
    {
    case TexInternalFormats::RED:  return 1; break;
    case TexInternalFormats::RG:   return 2; break;
    case TexInternalFormats::RGB:  return 3; break;
    case TexInternalFormats::RGBA: return 4; break;
    default: break;
    }
}

// OpenGL Texture
struct Texture : NonCopyable
{
public:
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

private:
    GLuint glHandle        =  0;
    int    width           =  0;
    int    height          =  0;
    int    boundSlot       = -1;
    TexInternalFormats internalFormat = TexInternalFormats::Unknown;
    fs::path _path;

public:
    constexpr Texture() = default;
    Texture(const Path& filePath) { loadFromFile(filePath); }

    Texture(Texture&& other) noexcept
    {
        reset();
        glHandle        = other.glHandle;
        other.glHandle  = 0;
        other.boundSlot = -1;

        width          = other.width;
        height         = other.height;
        boundSlot      = other.boundSlot;
        internalFormat = other.internalFormat;
        _path          = other._path;
        ASSERT(valid());
    }

    Texture& operator=(Texture&& other) noexcept
    {
        reset();
        glHandle        = other.glHandle;
        other.glHandle  = 0;
        other.boundSlot = -1;

        width          = other.width;
        height         = other.height;
        boundSlot      = other.boundSlot;
        internalFormat = other.internalFormat;
        _path          = other._path;
        ASSERT(valid());
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
            internalFormat  = TexInternalFormats::Unknown;
        }
    }

    void create()
    {
        reset();
        GL_CHECK(glGenTextures(1, &glHandle));
    }

    [[nodiscard]]
    bool created() const
    { return glHandle != 0 && SDL_GL_GetCurrentContext() != NULL; }

    bool valid() const
    {
        return
            width     >  0 &&
            height    >  0 &&
            glHandle  >  0 &&
            internalFormat != TexInternalFormats::Unknown;
    }

    bool loadFromFile(const Path& path)
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

    bool writeToFile(const Path& path)
    {
        ASSERT(created());
        bind();

        Vector2i size = getSize();
        int32_t  comp = 4;
        size_t   bufferSize = (size_t)size.x * size.y * comp;

        Vector<char> buffer;
        buffer.reserve(bufferSize);

        GL_CHECK(glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferSize, buffer.data()));
        stbi_write_png(path.string().c_str(), size.x, size.y, comp, buffer.data(), size.x * comp);
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
        const uint32_t           width,
        const uint32_t           height,
        const TexPixelFormats    format         = defaultFormat,
        const TexInternalFormats internalFormat = defaultInternalFormat,
        const bool               generateMipmap = true)
    {
        if (!created()) { create(); }

        this->width  = width;
        this->height = height;
        this->internalFormat = internalFormat;

        bind();
        setFilter(defaultTexFiltering);
        setUVMode(defaultUVMode);

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)format, GL_UNSIGNED_BYTE, data));
        if (generateMipmap)
        { GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D)); }
    }

    void setSubData(
        const TextureData&    data,
        const int32_t         xoffset, // where to put your data on the x
        const int32_t         yoffset, // where to put your data on the y
        const TexPixelFormats format = defaultFormat)  
    {
        setSubData(data.ptr(), data.width(), data.height(), xoffset, yoffset, format);
    }

    void setSubData(
        const void*           data,    // Your provided data
        const uint32_t        width,   // of your provided data
        const uint32_t        height,  // of your provided data
        const int32_t         xoffset, // where to put your data on the x
        const int32_t         yoffset, // where to put your data on the y
        const TexPixelFormats format = defaultFormat) // format of your provided data
    {
        int target = GL_TEXTURE_2D;
        int level  = 0;
        int type   = GL_UNSIGNED_BYTE;

        bind();
        GL_CHECK(glTexSubImage2D(target, level, xoffset, yoffset, width, height, (GLint)format, type, data));
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

    [[deprecated]]
    void setFilter(TextureFiltering min, TextureFiltering max)
    {
        bind();
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(min)) );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(max)) );
    }

    void setFilter(TextureMinFilter min, TextureMagFilter mag)
    {
        bind();
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(min)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mag)));
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

    void generateMipmaps()
    {
        bind();
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
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
    Rectf    rect;

    constexpr SubTexture() = default;
    SubTexture(Texture& tex, const Rectf& rect) : texture{ &tex }, rect{ rect } { }
    SubTexture(Texture& tex) : texture{ &tex }, rect{ Vector2f{0,0}, Vector2f(texture->getSize()) } { }
};