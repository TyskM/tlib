#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Texture.hpp"
#include "../DataStructures.hpp"
#include <map>

#pragma region Shaders

const char* TEXT_VERT_SHADER = R"""(
#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}  
)""";

const char* TEXT_FRAG_SHADER = R"""(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}  
)""";

#pragma endregion

struct Font
{
    static inline FT_Library _ft = nullptr;
    static void _init()
    {
        if (_ft != nullptr) return;
        if (FT_Init_FreeType(&_ft))
        { std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; }
    }

    struct Character
    {
        Texture      texture;  // ID handle of the glyph texture
        Vector2i     size;       // Size of glyph
        Vector2i     bearing;    // Offset from baseline to left/top of glyph
        unsigned int advance;    // Offset to advance to next glyph
    };

    std::map<char, Character> characters;

    void loadFont(const std::string& path, unsigned int size = 24)
    {
        _init();
        characters.clear();
        FT_Face face;
        FT_Error err = FT_New_Face(_ft, path.c_str(), 0, &face);
        if (err)
        {
            std::cout << "ERROR::FREETYPE: Failed to load font : Error code: " << err << std::endl;  
            return;
        }
        FT_Set_Pixel_Sizes(face, 0, size);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            Character ch;
            ch.texture.create();
            ch.texture.setData(
                face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                TextureFiltering::Linear, TexPixelFormats::RED, TexInternalFormats::RED);
            
            // now store character for later use
            ch.size = Vector2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            ch.bearing = Vector2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
            ch.advance = face->glyph->advance.x;
            
            characters.insert(std::pair<char, Character>(c, std::move(ch)));
        }

        FT_Done_Face(face);
    }
};
