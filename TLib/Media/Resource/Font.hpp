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

enum class FontRenderMode
{
    SDF    = FT_Render_Mode_::FT_RENDER_MODE_SDF,
    Normal = FT_Render_Mode_::FT_RENDER_MODE_NORMAL
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

struct Font : NonAssignable
{
protected:
    // DONE: a map isn't needed, use an array and instance by char. they're just numbers
    Texture textureAtlas;
    Vector<FontAtlasChar> characters;
    int32_t _newLineHeight = 0;
    uint32_t _size = 0;

public:
    [[nodiscard]]
    inline FontAtlasChar& getChar(size_t c)
    { return characters.at(c); }

    [[nodiscard]]
    inline bool containsChar(size_t c) const
    { return c >= 0 && c < characters.size(); }

    [[nodiscard]]
    inline FontAtlasChar& getFallbackChar()
    { return characters.back(); }

    [[nodiscard]]
    inline int32_t newLineHeight() const
    { return _newLineHeight; }

    [[nodiscard]]
    inline uint32_t size() const
    { return _size; }

    [[nodiscard]]
    inline bool created() const
    { return characters.size() > 0; }

    [[nodiscard]]
    Texture& getAtlas()
    { return textureAtlas; }

    [[nodiscard]]
    Vector2f calcTextSize(const String& text, float scale = 1.f)
    {
        Vector2f size;
        for (auto& strchar : text)
        {
            if (!containsChar(strchar))
            { continue; }

            FontAtlasChar& ch = characters.at(strchar);
            size.x += (ch.advance >> 6) * scale;
            if (ch.rect.getSize().y * scale > size.y)
            { size.y = ch.rect.getSize().y * scale; }
        }
        return size;
    }

    bool loadFromFile(
        const String&  path,
        unsigned int   size       = 24,
        size_t         rangeMin   = 0,
        size_t         rangeMax   = 128,
        FontRenderMode renderMode = FontRenderMode::SDF)
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
        FT_Bitmap* bmp = &slot->bitmap;

        size_t charCount = FontDetail::getGlyphCount(face);
        charCount = std::min(charCount, rangeMax);
        characters.resize(charCount);
        _newLineHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
        _size = size;

        // DONE: Pack glyphs better, theres tons of wasted space!!!
        // Used rectpack2d for nice packing :)
        // DONE: speed up loading times somehow
        // stopped loading non existent characters for big speed up
        // it's still pretty slow
        // Profiler says most of the time is spent in FT_Render_Glyph
        // Started copying the first call to FT_Render_Glyph and using that for second iteration
        // It's not painfully slow anymore yeehaw

        ///// Loop through chars once to find minimum size needed to pack everything
        const auto maxTexSize = Renderer::getMaxTextureSize();
        const auto discard_step = -4;
        using spaces_type = rectpack2D::empty_spaces<false>;
        using rect_type = output_rect_t<spaces_type>;

        auto report_successful   = [](rect_type&) { return callback_result::CONTINUE_PACKING; };
        auto report_unsuccessful = [](rect_type&) { return callback_result::ABORT_PACKING; };

        std::vector<rect_type> rects;
        rects.reserve(charCount);
        Vector<Vector<char>> sdfBitmapBuffer;
        sdfBitmapBuffer.reserve(charCount);

        for (size_t c = rangeMin; c < charCount; c++)
        {
            if (!FT_Get_Char_Index(face, c))
            {
                rects.emplace_back(rect_xywh());
                sdfBitmapBuffer.emplace_back();
                continue;
            }

            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT))
            { tlog::error("Failed to load character '{}' from font file '{}'", c, path); continue; }

            FT_Render_Glyph(slot, static_cast<FT_Render_Mode_>(renderMode));
            
            // Save glyph rect for packing, and save bitmap buffer for second iteration
            rects.emplace_back(rect_xywh(0, 0, bmp->width, bmp->rows));
            sdfBitmapBuffer.emplace_back(bmp->buffer, bmp->buffer + bmp->width * bmp->rows);

            //// Write their info into the char map so we can actually use them
            FontAtlasChar& ch = characters.at(c);
            ch.bearing        = Vector2i(slot->bitmap_left, slot->bitmap_top);
            ch.advance        = slot->advance.x;
            ch.rect.width     = slot->bitmap.width;
            ch.rect.height    = slot->bitmap.rows;
        }

        /// Save a fallback character
        // 0 is null https://www.cs.cmu.edu/~pattis/15-1XX/common/handouts/ascii.html
        FT_Load_Char(face, 0, FT_LOAD_DEFAULT);
        FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
        rects.emplace_back(rect_xywh(0, 0, bmp->width, bmp->rows));
        sdfBitmapBuffer.emplace_back(bmp->buffer, bmp->buffer + bmp->width * bmp->rows);
        FontAtlasChar& ch = characters.emplace_back();
        ch.bearing        = Vector2i(slot->bitmap_left, slot->bitmap_top);
        ch.advance        = slot->advance.x;
        ch.rect.width     = slot->bitmap.width;
        ch.rect.height    = slot->bitmap.rows;

        const auto result_size = find_best_packing<spaces_type>(rects,
            make_finder_input(maxTexSize, discard_step, report_successful, report_unsuccessful, flipping_option::DISABLED));

        // Surely this wont happen... surely?
        if (result_size.w > maxTexSize || result_size.h > maxTexSize)
        {
            tlog::critical("The font '{}' is too large for a texture atlas! try reducing the size or range.", path);
            return false;
        }

        //// Create texture using the size we found
        textureAtlas.create();
        textureAtlas.setData(NULL, result_size.w, result_size.h, TexPixelFormats::RED, TexInternalFormats::RED);
        textureAtlas.setUnpackAlignment(1);

        //// Loop through saved sdf bitmaps,
        //// load them into GPU texture using the positions we found with rectpack2d
        for (size_t i = 0; i < sdfBitmapBuffer.size(); i++)
        {
            auto& bitmap = sdfBitmapBuffer[i];
            if (bitmap.empty()) { continue; }

            FontAtlasChar& ch = characters.at(i);

            auto x = rects[i].x;
            auto y = rects[i].y;

            textureAtlas.setSubData(bitmap.data(), ch.rect.width, ch.rect.height,
                x, y, TexPixelFormats::RED);

            // Write their assigned positions
            ch.rect.x = x;
            ch.rect.y = y;
        }

        // All done, yeehaw
        FT_Done_Face(face);
        return true;
    }
};
