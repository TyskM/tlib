#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stb_truetype.h>

#include "Texture.hpp"
#include "../DataStructures.hpp"
#include <map>

struct FontCharacter
{
    Texture      texture;
    Vector2i     size;       // Size of glyph
    Vector2i     bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

struct Font
{
private:
    static inline FT_Library _ft = nullptr;
    static void _init()
    {
        if (_ft != nullptr) return;
        if (FT_Init_FreeType(&_ft))
        { std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; }
    }

    std::map<char, FontCharacter> characters;

public:
    inline FontCharacter& getChar(const char c)
    { return characters.at(c); }

    inline bool containsChar(const char c) const
    { return characters.contains(c); }

    Vector2f calcTextSize(const String& text, float scale = 1.f)
    {
        Vector2f size;
        for (auto& strchar : text)
        {
            if (!characters.contains(strchar))
            { std::cout << "Font does not contain character \"" << strchar << "\"" << std::endl; continue; }

            FontCharacter& ch = characters.at(strchar);
            size.x += (ch.advance >> 6) * scale;
            if (ch.size.y * scale > size.y)
            { size.y = ch.size.y * scale; }
        }
        return size;
    }

    void loadFontSdf(const String& path, unsigned int size = 24, TextureFiltering filtering = Texture::defaultTexFiltering)
    {
        _init();
        characters.clear();
        FT_Face _face;
        FT_Error err = FT_New_Face(_ft, path.c_str(), 0, &_face);
        if (err)
        {
            tlog::error("FREETYPE: Failed to load font '{}' Error code: {}", path, err);
            return;
        }
        FT_Set_Pixel_Sizes(_face, 0, size);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        FT_GlyphSlot slot = _face->glyph;

        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(_face, c, FT_LOAD_RENDER))
            {   tlog::error("Failed to load character '{}' from font file '{}'", c, path);
                continue; }

            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);

            FontCharacter& ch = characters[c];
            ch.texture.create();
            ch.texture.setUnpackAlignment(1);
            ch.texture.setData(
                slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows,
                filtering, TexPixelFormats::RED, TexInternalFormats::RED);

            // now store character for later use
            ch.size = Vector2i(slot->bitmap.width, slot->bitmap.rows);
            ch.bearing = Vector2i(slot->bitmap_left, slot->bitmap_top);
            ch.advance = slot->advance.x;
        }
    }

    void loadFont(const String& path, unsigned int size = 24, TextureFiltering filtering = Texture::defaultTexFiltering)
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

        for (unsigned char c = 0; c < 128; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            FontCharacter& ch = characters[c];
            ch.texture.create();
            ch.texture.setUnpackAlignment(1);
            ch.texture.setData(
                face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                filtering, TexPixelFormats::RED, TexInternalFormats::RED, true);
            
            // now store character for later use
            ch.size = Vector2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            ch.bearing = Vector2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
            ch.advance = face->glyph->advance.x;
        }

        FT_Done_Face(face);
    }
};
