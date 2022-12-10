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

enum class TextureFiltering
{
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR
};

struct Texture : NonAssignable
{
    static inline GLubyte fallbackImage[2][2][4] =
    {
        { {255, 0, 255, 255}, {0,   0, 0,   255} },
        { {0,   0, 0,   255}, {255, 0, 255, 255} }
    };

    GLuint glHandle = 0;
    int width, height;

    Texture() { }
    Texture(const std::string& filePath, TextureFiltering texFiltering = TextureFiltering::Linear)
    {
        loadFromFile(filePath, texFiltering);
    }

    ~Texture()
    {
        if (created())
        {
            GL_CHECK(glDeleteTextures(1, &glHandle));
        }
    }

    void create(Vector2i size, TextureFiltering texFiltering = TextureFiltering::Linear)
    {
        init();
        bind();
        
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(texFiltering)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(texFiltering)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        width = size.x; height = size.y;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        unbind();
    }

    void loadFromFile(std::string filePath, TextureFiltering texFiltering = TextureFiltering::Linear)
    {
        init();

        int nrChannels;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

        bind();

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(texFiltering)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(texFiltering)));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        if (stbi_failure_reason() || !data/* || data[0] == '\0'*/)
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
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
            GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

            stbi_image_free(data);
        }
        unbind();
    }

    void bind()
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, glHandle));
    }

    static inline void unbind()
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }

    bool created()
    { return glHandle != 0 && SDL_GL_GetCurrentContext() != NULL; }

protected:
    void loadFallbackTexture()
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallbackImage));
    }

    void init()
    {
        if (!created())
        { GL_CHECK(glGenTextures(1, &glHandle)); }
    }
};