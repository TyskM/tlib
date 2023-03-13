#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Texture.hpp"
#include <TLib/DataStructures.hpp>
#include <map>

struct FontCharacter
{
    Texture      texture;
    Vector2i     size;       // Size of glyph
    Vector2i     bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

struct FontDetail
{
    // TODO: a map isn't needed, use an array and instance by char. they're just numbers
    using CharMap = std::map<char, FontCharacter>;

    static inline FT_Library ft = nullptr;
    static void initFreetype()
    {
        if (ft != nullptr) return;
        if (FT_Init_FreeType(&ft))
        { tlog::error("FREETYPE: Could not init FreeType Library"); }
    }
};

struct FontBase
{
protected:
    FontDetail::CharMap characters;
    int32_t _newLineHeight = 0;
    int32_t _height = 0;

public:
    [[nodiscard]]
    inline FontCharacter& getChar(const char c)
    { return characters.at(c); }

    [[nodiscard]]
    inline bool containsChar(const char c) const
    { return characters.contains(c); }

    [[nodiscard]]
    inline int32_t height() const
    { return _height; }

    [[nodiscard]]
    inline int32_t newLineHeight() const
    { return _newLineHeight; }

    [[nodiscard]]
    inline bool created() const
    { return characters.size() > 0; }

    [[nodiscard]]
    Vector2f calcTextSize(const String& text, float scale = 1.f)
    {
        Vector2f size;
        for (auto& strchar : text)
        {
            if (!characters.contains(strchar))
            { tlog::error(("Font does not contain character \"{}\""), strchar); continue; }

            FontCharacter& ch = characters.at(strchar);
            size.x += (ch.advance >> 6) * scale;
            if (ch.size.y * scale > size.y)
            { size.y = ch.size.y * scale; }
        }
        return size;
    }
};

struct SDFFont : FontBase
{
    bool loadFromFile(const String& path, unsigned int size = 24)
    {
        FontDetail::initFreetype();
        characters.clear();
        FT_Face face;
        FT_Error err = FT_New_Face(FontDetail::ft, path.c_str(), 0, &face);
        if (err)
        {
            tlog::error("FREETYPE: Failed to load font '{}' Error code: {}", path, err);
            return false;
        }
        FT_Set_Pixel_Sizes(face, 0, size);

        _newLineHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

        FT_GlyphSlot slot = face->glyph;

        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                tlog::error("Failed to load character '{}' from font file '{}'", c, path);
                continue;
            }

            

            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
            
            // Bitmap.rows in bitmap height
            if (slot->bitmap.rows > _height)
            { _height = slot->bitmap.rows; }

            FontCharacter& ch = characters[c];
            ch.texture.create();
            ch.texture.setUnpackAlignment(1);
            ch.texture.setData(
                slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows,
                TexPixelFormats::RED, TexInternalFormats::RED);

            // now store character for later use
            ch.size = Vector2i(slot->bitmap.width, slot->bitmap.rows);
            ch.bearing = Vector2i(slot->bitmap_left, slot->bitmap_top);
            ch.advance = slot->advance.x;
        }

        FT_Done_Face(face);
        return true;
    }
};

struct BitmapFont : FontBase
{
    bool loadFromFile(const String& path, unsigned int size = 24)
    {
        FontDetail::initFreetype();
        characters.clear();
        FT_Face face;
        FT_Error err = FT_New_Face(FontDetail::ft, path.c_str(), 0, &face);
        if (err)
        {
            tlog::error("FREETYPE: Failed to load font : Error code: {}", err);
            return false;
        }
        FT_Set_Pixel_Sizes(face, 0, size);

        _newLineHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

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
                TexPixelFormats::RED, TexInternalFormats::RED, true);

            ch.size = Vector2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            ch.bearing = Vector2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
            ch.advance = face->glyph->advance.x;
        }

        FT_Done_Face(face);
        return true;
    }
};
