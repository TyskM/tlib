#pragma once

#define NOMINMAX
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <string>
#include "GLHelpers.hpp"
#include "../Files.hpp"
#include "../NonAssignable.hpp"
#include "../Macros.hpp"
#include "GLState.hpp"

enum class TextureFiltering
{
    Unknown = -1,
    Nearest = GL_NEAREST,
    Linear  = GL_LINEAR
};

// TODO: add the rest of the formats
enum class TexInternalFormats : int
{
    Unknown         = -1,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   = GL_DEPTH_STENCIL,
    RED  = GL_RED,
    RG   = GL_RG,
    RGB  = GL_RGB,
    RGBA = GL_RGBA
};

enum class TexPixelFormats : int
{
    Unknown         = -1,
    RED             = GL_RED,
    RG              = GL_RG,
    RGB             = GL_RGB,
    BGR             = GL_BGR,
    RGBA            = GL_RGBA,
    BGRA            = GL_BGRA,
    RED_INTEGER     = GL_RED_INTEGER,
    RG_INTEGER      = GL_RG_INTEGER,
    RGB_INTEGER     = GL_RGB_INTEGER,
    BGR_INTEGER     = GL_BGR_INTEGER,
    RGBA_INTEGER    = GL_RGBA_INTEGER,
    BGRA_INTEGER    = GL_BGRA_INTEGER,
    STENCIL_INDEX   = GL_STENCIL_INDEX,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   = GL_DEPTH_STENCIL
};

struct Texture : NonCopyable
{
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    static constexpr TexPixelFormats defaultFormat = TexPixelFormats::RGBA;
    static constexpr TexInternalFormats defaultInternalFormat = TexInternalFormats::RGBA;
    static constexpr GLubyte fallbackImage[2][2][4] =
    {
        { {255, 0, 255, 255}, {0,   0, 0,   255} },
        { {0,   0, 0,   255}, {255, 0, 255, 255} }
    };

    GLuint glHandle = 0;
    int width  = 0;
    int height = 0;
    TextureFiltering filtering        = TextureFiltering::Unknown;
    TexPixelFormats format            = TexPixelFormats::Unknown;
    TexInternalFormats internalFormat = TexInternalFormats::Unknown;

    Texture() { }
    Texture(const std::string& filePath, TextureFiltering texFiltering = TextureFiltering::Linear)
    {
        loadFromFile(filePath, texFiltering);
    }

    ~Texture()
    {
        reset();
    }

    Texture& operator=(Texture&& other) noexcept
    {
        reset();
        glHandle       = other.glHandle;
        other.glHandle = 0;
        width          = other.width;
        height         = other.height;
        filtering      = other.filtering;
        format         = other.format;
        internalFormat = other.internalFormat;
        return *this;
    }

    Texture(Texture&& other) noexcept
    {
        reset();
        glHandle       = other.glHandle;
        other.glHandle = 0;
        width          = other.width;
        height         = other.height;
        filtering      = other.filtering;
        format         = other.format;
        internalFormat = other.internalFormat;
    }

    void reset()
    {
        if (created())
        {
            GL_CHECK(glDeleteTextures(1, &glHandle));
            glHandle       = 0;
            width          = 0;
            height         = 0;
            filtering      = TextureFiltering::Unknown;
            format         = TexPixelFormats::Unknown;
            internalFormat = TexInternalFormats::Unknown;
        }
    }

    void create()
    {
        reset();
        GL_CHECK(glGenTextures(1, &glHandle));
    }

    void loadFromFile(std::string filePath, TextureFiltering texFiltering = TextureFiltering::Nearest)
    {
        int nrChannels;
        stbi_uc* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 4);

        if (data == NULL) // Error!!
        {
            std::cerr << "!!!ERROR: Failed to load image from path (\"" << filePath << "\")\n";

            if (stbi_failure_reason())
            { std::cerr << "\tReason: " << stbi_failure_reason() << "\n"; }
            
            stbi_image_free(data);
            loadFallbackTexture();
        }
        else
        {
            // TODO: Support different formats?
            // https://stackoverflow.com/questions/23150123/loading-png-with-stb-image-for-opengl-texture-gives-wrong-colors
            setData(data, width, height, texFiltering, defaultFormat, defaultInternalFormat);

            stbi_image_free(data);
        }
        
    }

    void setData(
        unsigned char* data, int width, int height,
        TextureFiltering texFiltering = TextureFiltering::Linear,
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
        unbind();
    }

    void bind()
    {
        if (glState.boundTexture == this) { return; }
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, glHandle));
        glState.boundTexture = this;
    }

    static inline void unbind()
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
        glState.boundTexture = nullptr;
    }

    bool created()
    { return glHandle != 0 && SDL_GL_GetCurrentContext() != NULL; }


    Vector2i getSize()
    { return Vector2i{ width, height }; }

    void loadFallbackTexture()
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, (GLint)defaultInternalFormat, 2, 2, 0, (GLenum)defaultFormat, GL_UNSIGNED_BYTE, fallbackImage));
    }
};

