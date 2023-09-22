#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/thirdparty/rectpack2d/empty_spaces.h>
#include <TLib/thirdparty/rectpack2d/empty_space_allocators.h>
#include <TLib/thirdparty/rectpack2d/rect_structs.h>
#include <TLib/thirdparty/rectpack2d/best_bin_finder.h>
#include <TLib/thirdparty/rectpack2d/finders_interface.h>
using namespace rectpack2D;

struct FontChar
{
    Texture      texture;
    Vector2i     size;       // Size of glyph
    Vector2i     bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

struct FontAtlasChar
{
    Recti    rect;
    Vector2i bearing;
    uint32_t advance;
};

struct FontDetail
{
    static inline FT_Library ft = nullptr;
    static void initFreetype()
    {
        if (ft != nullptr) return;
        if (FT_Init_FreeType(&ft))
        { tlog::error("FREETYPE: Could not init FreeType Library"); }
    }

    static size_t getGlyphCount(FT_Face& face)
    {
        FT_UInt index;
        FT_ULong character = FT_Get_First_Char(face, &index);
        size_t ret = 0;

        while (true) {
            character = FT_Get_Next_Char(face, character, &index);
            ++ret;
            if (!index) break;
        }
        return ret;
    }
};

struct FontBase : NonAssignable
{
protected:
    // DONE: a map isn't needed, use an array and instance by char. they're just numbers
    Texture textureAtlas;
    Vector<FontAtlasChar> characters;
    int32_t _newLineHeight = 0;

public:
    [[nodiscard]]
    inline FontAtlasChar& getChar(size_t c)
    { return characters.at(c); }

    [[nodiscard]]
    inline bool containsChar(size_t c) const
    { return c >= 0 && c < characters.size(); }

    [[nodiscard]]
    inline int32_t newLineHeight() const
    { return _newLineHeight; }

    [[nodiscard]]
    inline bool created() const
    { return characters.size() > 0; }

    Texture& getAtlas()
    { return textureAtlas; }

    [[nodiscard]]
    Vector2f calcTextSize(const String& text, float scale = 1.f)
    {
        Vector2f size;
        for (auto& strchar : text)
        {
            if (!containsChar(strchar))
            { tlog::error(("Font does not contain character \"{}\""), strchar); continue; }

            FontAtlasChar& ch = characters.at(strchar);
            size.x += (ch.advance >> 6) * scale;
            if (ch.rect.getSize().y * scale > size.y)
            { size.y = ch.rect.getSize().y * scale; }
        }
        return size;
    }
};

struct SDFFont : FontBase
{
    bool loadFromFile(const String& path, unsigned int size = 24, size_t rangeMin = 0, size_t rangeMax = SIZE_MAX)
    {
        ///// Load font and cache basic details
        FontDetail::initFreetype();
        characters.clear();
        FT_Face face;
        FT_Error err = FT_New_Face(FontDetail::ft, path.c_str(), 0, &face);
        if (err)
        {
            tlog::error("FREETYPE: Failed to load font '{}' Error code: {}", path, err);
            return false;
        }
        FT_Select_Charmap(face, ft_encoding_unicode);
        FT_Set_Pixel_Sizes(face, 0, size);
        FT_GlyphSlot slot = face->glyph;

        size_t charCount = FontDetail::getGlyphCount(face);
        charCount = std::min(charCount, rangeMax);
        characters.resize(charCount);
        _newLineHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

        // DONE: Pack glyphs better, theres tons of wasted space!!!
        // Used rectpack2d for nice packing :)
        // TODO: speed up loading times somehow
        // stopped loading non existent characters for big speed up
        // it's still pretty slow
        // Profiler says most of the time is spent in FT_Render_Glyph

        ///// Loop through chars once to find minimum size needed to pack everything
        const auto maxTexSize = Renderer::getMaxTextureSize();
        const auto discard_step = -4;
        using spaces_type = rectpack2D::empty_spaces<false>;
        using rect_type = output_rect_t<spaces_type>;

        auto report_successful   = [](rect_type&) { return callback_result::CONTINUE_PACKING; };
        auto report_unsuccessful = [](rect_type&) { return callback_result::ABORT_PACKING; };

        std::vector<rect_type> rects;
        rects.reserve(charCount);
        for (size_t c = rangeMin; c < charCount; c++)
        {
            if (!FT_Get_Char_Index(face, c))
            { rects.emplace_back(rect_xywh()); continue; }

            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT))
            { tlog::error("Failed to load character '{}' from font file '{}'", c, path); continue; }
            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
            FT_Bitmap* bmp = &slot->bitmap;
            rects.emplace_back(rect_xywh(0, 0, bmp->width, bmp->rows));
        }
        const auto result_size = find_best_packing<spaces_type>(rects,
            make_finder_input(maxTexSize, discard_step, report_successful, report_unsuccessful, flipping_option::DISABLED));

        if (result_size.w > maxTexSize || result_size.h > maxTexSize)
        {
            tlog::critical("The font '{}' is too large for a texture atlas! try reducing the size or range.", path);
            return false;
        }

        //// Create texture using the size we found
        textureAtlas.create();
        textureAtlas.setData(NULL, result_size.w, result_size.h, TexPixelFormats::RED, TexInternalFormats::RED);
        textureAtlas.setUnpackAlignment(1);
        
        //// Loop through chars again,
        //// load them into GPU texture using the positions we found with rectpack2d
        for (size_t c = rangeMin; c < charCount; c++)
        {
            if (!FT_Get_Char_Index(face, c)) { continue; }

            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT))
            { tlog::error("Failed to load character '{}' from font file '{}'", c, path); continue; }
            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
            FT_Bitmap* bmp = &slot->bitmap;

            auto x = rects[c].x;
            auto y = rects[c].y;

            textureAtlas.setSubData(bmp->buffer, bmp->width, bmp->rows,
                x, y, TexPixelFormats::RED);

            //// Write their info into the char map so we can actually use them
            FontAtlasChar& ch = characters.at(c);
            ch.rect    = {Vector2i(x, y), Vector2i(bmp->width, bmp->rows)};
            ch.bearing = Vector2i(slot->bitmap_left, slot->bitmap_top);
            ch.advance = slot->advance.x;
        }

        // All done, yeehaw
        FT_Done_Face(face);
        return true;
    }
};

//struct BitmapFont : FontBase
//{
//    bool loadFromFile(const String& path, unsigned int size = 24)
//    {
//        FontDetail::initFreetype();
//        characters.clear();
//        FT_Face face;
//        FT_Error err = FT_New_Face(FontDetail::ft, path.c_str(), 0, &face);
//        if (err)
//        {
//            tlog::error("FREETYPE: Failed to load font : Error code: {}", err);
//            return false;
//        }
//        FT_Set_Pixel_Sizes(face, 0, size);
//
//        _newLineHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
//
//        // HACK: load all chars from font
//        size_t charCount = 128;
//        characters.resize(charCount);
//        for (unsigned char c = 0; c < charCount; c++)
//        {
//            // load character glyph 
//            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//            {
//                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//                continue;
//            }
//
//            FontCharacter& ch = characters.at(c);
//            ch.texture.create();
//            ch.texture.setUnpackAlignment(1);
//            ch.texture.setData(
//                face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows,
//                TexPixelFormats::RED, TexInternalFormats::RED, true);
//
//            ch.size = Vector2i(face->glyph->bitmap.width, face->glyph->bitmap.rows);
//            ch.bearing = Vector2i(face->glyph->bitmap_left, face->glyph->bitmap_top);
//            ch.advance = face->glyph->advance.x;
//        }
//
//        FT_Done_Face(face);
//        return true;
//    }
//};
