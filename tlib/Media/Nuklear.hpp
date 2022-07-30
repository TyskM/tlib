
// This is scuffed don't use

#pragma once
#pragma warning(disable:4996) // Disables _CRT_SECURE_NO_WARNINGS

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#include <nuklear/nuklear.h>

#include <SDL2/SDL.h>
#include "Renderer.hpp"
#include "Texture.hpp"
#include "Font.hpp"
#include <vector>
#include <string.h>

struct NuklearWrapper
{
    SDL_Window* win;
    Renderer* renderer;

    const int MAX_VERTEX_MEMORY = 512 * 1024;
    const int MAX_ELEMENT_MEMORY = 128 * 1024;
    size_t text_max = 256;
    nk_context context;
    struct nk_buffer cmds;
    // font data
    nk_font_atlas atlas;
    struct nk_user_font *user_font;
    int font_height;

    nk_user_font font;
    FC_Font* ttfFont = nullptr;

    NuklearWrapper() = default;
    NuklearWrapper(SDL_Window& win, Renderer& renderer, FC_Font& fcfont)
    {
        create(win, renderer, fcfont);
    }

    ~NuklearWrapper()
    {
        nk_free(&context);
        nk_buffer_free(&cmds);
    }

    void create(SDL_Window& win, Renderer& renderer, FC_Font& fcfont)
    {
        this->win = &win;
        this->renderer = &renderer;
        this->ttfFont = &fcfont;

        font.userdata = nk_handle_ptr(ttfFont);
        font.height = FC_GetLineHeight(ttfFont);
        font.width = nk_sdl_font_get_text_width;

        nk_init_default(&context, &font);
        nk_buffer_init_default(&cmds);

        context.clip.copy = nk_sdl_clipboard_copy;
        context.clip.paste = nk_sdl_clipboard_paste;
        context.clip.userdata = nk_handle_ptr(0);
    }

    operator nk_context*() { return &context; }
    operator nk_context&() { return context; }

    // IMPL

    // https://stackoverflow.com/questions/29061505/where-can-i-get-correct-pitch-parameter-for-sdl-renderreadpixels-function-in-sdl
    static constexpr int calcPitch(Uint32 format, int width) noexcept
    {
        int pitch;

        if (SDL_ISPIXELFORMAT_FOURCC(format) || SDL_BITSPERPIXEL(format) >= 8)
        { pitch = (width * SDL_BYTESPERPIXEL(format)); }
        else
        { pitch = ((width * SDL_BITSPERPIXEL(format)) + 7) / 8; }
        pitch = (pitch + 3) & ~3; /* 4-byte aligning for speed */
        return pitch;
    }

    void sdl_draw_image(const struct nk_command_image* image, int x, int y, int w, int h)
    {
        SDL_Texture* tex = static_cast<SDL_Texture*>(image->img.handle.ptr);
        SDL_Rect srcrect = { image->img.region[0], image->img.region[1], image->img.region[2], image->img.region[3] };
        SDL_Rect dstrect = { x, y, w, h };
        SDL_RenderCopy(*renderer, tex, nk_image_is_subimage(&image->img) ? &srcrect : NULL, &dstrect);
    }

    static void nk_sdl_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
    {
        const char *text = SDL_GetClipboardText();
        if (text) nk_textedit_paste(edit, text, nk_strlen(text));
        (void)usr;
    }

    static void nk_sdl_clipboard_copy(nk_handle usr, const char *text, int len)
    {
        char *str = 0;
        (void)usr;
        if (!len) return;
        str = (char*)malloc((size_t)len+1);
        if (!str) return;
        memcpy(str, text, (size_t)len);
        str[len] = '\0';
        SDL_SetClipboardText(str);
        free(str);
    }

    static float nk_sdl_font_get_text_width(nk_handle handle, float height, const char *text, int len)
    {
        FC_Font* font = static_cast<FC_Font*>(handle.ptr);
        if (!font || !text) { return 0; }

        return static_cast<float>( getTextWidth(font, text, len) );
        
        // TODO: prolly a bug above
        // Should prolly modify getTextWidth to take a length param
        // instead of doing this junk \/

        //Font* font = (Font*)handle.ptr;
        //if (!font || !text) { return 0; }

        //std::vector<char> tmp_buffer;
        //tmp_buffer.resize(len + 1);

        //strncpy(tmp_buffer.data(), text, len);
        //tmp_buffer[len] = '\0';

        //int w, h;
        //return static_cast<float>( getTextWidth(*font, tmp_buffer.data()) );
    }

    static Uint16 getTextWidth(FC_Font* font, const char* text, int len)
    {
        if(text == NULL || font == NULL) return 0;

        const char* c;
        Uint16 width = 0;
        Uint16 bigWidth = 0;  // Allows for multi-line strings
        size_t i = 0;

        for (c = text; *c != '\0' && i < len; c++)
        {
            ++i;
            //if(*c == '\n')
            //{
            //    bigWidth = bigWidth >= width ? bigWidth : width;
            //    width = 0;
            //    continue;
            //}

            FC_GlyphData glyph;
            Uint32 codepoint = FC_GetCodepointFromUTF8(&c, 1);
            if(FC_GetGlyphData(font, &glyph, codepoint) || FC_GetGlyphData(font, &glyph, ' '))
                width += glyph.rect.w;
        }
        bigWidth = bigWidth >= width? bigWidth : width;

        return bigWidth;
    }


    // API

    void handleEvent(SDL_Event* evt)
    {    
        if (context.input.mouse.grab)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            context.input.mouse.grab = 0;
        }
        else if (context.input.mouse.ungrab)
        {
            int x = context.input.mouse.prev.x, y = context.input.mouse.prev.y;
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_WarpMouseInWindow(win, x, y);
            context.input.mouse.ungrab = 0;
        }
        if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN)
        {
            int down = evt->type == SDL_KEYDOWN;
            const Uint8* state = SDL_GetKeyboardState(0);
            SDL_Keycode sym = evt->key.keysym.sym;

            // TODO: Convert to switch
            if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT) nk_input_key(&context, NK_KEY_SHIFT, down);
            else if (sym == SDLK_DELETE) nk_input_key(&context, NK_KEY_DEL, down);
            else if (sym == SDLK_RETURN) nk_input_key(&context, NK_KEY_ENTER, down);
            else if (sym == SDLK_TAB) nk_input_key(&context, NK_KEY_TAB, down);
            else if (sym == SDLK_BACKSPACE) nk_input_key(&context, NK_KEY_BACKSPACE, down);
            else if (sym == SDLK_HOME)
            {
                nk_input_key(&context, NK_KEY_TEXT_START, down);
                nk_input_key(&context, NK_KEY_SCROLL_START, down);
            }
            else if (sym == SDLK_END)
            {
                nk_input_key(&context, NK_KEY_TEXT_END, down);
                nk_input_key(&context, NK_KEY_SCROLL_END, down);
            }
            else if (sym == SDLK_PAGEDOWN)
            {
                nk_input_key(&context, NK_KEY_SCROLL_DOWN, down);
            }
            else if (sym == SDLK_PAGEUP)
            {
                nk_input_key(&context, NK_KEY_SCROLL_UP, down);
            }
            else if (sym == SDLK_z) nk_input_key(&context, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_r) nk_input_key(&context, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_c) nk_input_key(&context, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_v) nk_input_key(&context, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_x) nk_input_key(&context, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_b) nk_input_key(&context, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_e) nk_input_key(&context, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
            else if (sym == SDLK_UP) nk_input_key(&context, NK_KEY_UP, down);
            else if (sym == SDLK_DOWN) nk_input_key(&context, NK_KEY_DOWN, down);
            else if (sym == SDLK_LEFT)
            {
                if (state[SDL_SCANCODE_LCTRL]) nk_input_key(&context, NK_KEY_TEXT_WORD_LEFT, down);
                else nk_input_key(&context, NK_KEY_LEFT, down);
            }
            else if (sym == SDLK_RIGHT)
            {
                if (state[SDL_SCANCODE_LCTRL]) nk_input_key(&context, NK_KEY_TEXT_WORD_RIGHT, down);
                else nk_input_key(&context, NK_KEY_RIGHT, down);
            }
        }
        else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP)
        {
            /* mouse button */
            int down = evt->type == SDL_MOUSEBUTTONDOWN;
            const int x = evt->button.x, y = evt->button.y;
            if (evt->button.button == SDL_BUTTON_LEFT) {
                if (evt->button.clicks > 1)
                { nk_input_button(&context, NK_BUTTON_DOUBLE, x, y, down); }
                nk_input_button(&context, NK_BUTTON_LEFT, x, y, down);
            }
            else if (evt->button.button == SDL_BUTTON_MIDDLE) nk_input_button(&context, NK_BUTTON_MIDDLE, x, y, down);
            else if (evt->button.button == SDL_BUTTON_RIGHT) nk_input_button(&context, NK_BUTTON_RIGHT, x, y, down);
        }
        else if (evt->type == SDL_MOUSEMOTION) {
            /* mouse motion */
            if (context.input.mouse.grabbed)
            {
                int x = context.input.mouse.prev.x, y = context.input.mouse.prev.y;
                nk_input_motion(&context, x + evt->motion.xrel, y + evt->motion.yrel);
            }
            else nk_input_motion(&context, evt->motion.x, evt->motion.y);
        }
        else if (evt->type == SDL_TEXTINPUT)
        {
            nk_glyph glyph;
            memcpy(glyph, evt->text.text, NK_UTF_SIZE);
            nk_input_glyph(&context, glyph);
        }
        else if (evt->type == SDL_MOUSEWHEEL)
        {
            nk_input_scroll(&context, nk_vec2(evt->wheel.x, evt->wheel.y));
        }
    }    

    void render()
    {    
        const struct nk_command *cmd;
        nk_foreach(cmd, &context)
        {
            switch (cmd->type)
            {
            case NK_COMMAND_NOP: { break; }
            case NK_COMMAND_SCISSOR:
            {
                auto s = reinterpret_cast<const nk_command_scissor*>(cmd);
                SDL_Rect rect = { s->x, s->y, s->w, s->h };
                SDL_RenderSetClipRect(renderer->_renderer, &rect );
                break;
            }   
            case NK_COMMAND_LINE:
            {
                auto l = reinterpret_cast<const struct nk_command_line*>(cmd);
                renderer->drawLine(l->begin, l->end, { l->color.r, l->color.g, l->color.b, l->color.a });
                break;
            }
            case NK_COMMAND_RECT:
            {
                auto r = reinterpret_cast<const struct nk_command_rect*>(cmd);
                renderer->drawRoundedRect(r->x, r->y, r->w, r->h, r->rounding, { r->color.r, r->color.g, r->color.b, r->color.a });
                break;
            }
            case NK_COMMAND_RECT_FILLED:
            {
                auto r = reinterpret_cast<const struct nk_command_rect_filled*>(cmd);
                renderer->drawRoundedRectFilled(r->x, r->y, r->w, r->h, r->rounding, { r->color.r, r->color.g, r->color.b, r->color.a });
                break;
            }
            case NK_COMMAND_CIRCLE:
            {
                auto c = reinterpret_cast<const struct nk_command_circle*>(cmd);
                int xr, yr;
                xr = c->w/2;
                yr = c->h/2;
                renderer->drawEllipse(c->x + xr, c->y + yr, xr, yr, { c->color.r, c->color.g, c->color.b, c->color.a });
                break;
            }
            case NK_COMMAND_CIRCLE_FILLED:
            {
                auto c = reinterpret_cast<const struct nk_command_circle_filled*>(cmd);
                int xr, yr;
                xr = c->w/2;
                yr = c->h/2;
                renderer->drawEllipseFilled(c->x + xr, c->y + yr, xr, yr, { c->color.r, c->color.g, c->color.b, c->color.a });
                break;
            }
            case NK_COMMAND_TRIANGLE:
            {
                auto t = reinterpret_cast<const struct nk_command_triangle*>(cmd);
                renderer->drawTriangle(t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, { t->color.r, t->color.g, t->color.b, t->color.a });
                break;
            }
            case NK_COMMAND_TRIANGLE_FILLED:
            {
                auto t = reinterpret_cast<const struct nk_command_triangle_filled*>(cmd);
                renderer->drawTriangleFilled(t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, { t->color.r, t->color.g, t->color.b, t->color.a });
                break;
            }
            case NK_COMMAND_POLYGON:
            {
                auto p = reinterpret_cast<const struct nk_command_polygon*>(cmd);
                renderer->drawPolygon(p->points, p->point_count, { p->color.r, p->color.g, p->color.b, p->color.a });
                break;
            }
            case NK_COMMAND_POLYGON_FILLED:
            {
                auto p = reinterpret_cast<const struct nk_command_polygon_filled*>(cmd);
                renderer->drawPolygon(p->points, p->point_count, { p->color.r, p->color.g, p->color.b, p->color.a });
                break;
            }
            case NK_COMMAND_POLYLINE: { break; }
            case NK_COMMAND_TEXT:
            {
                auto t = reinterpret_cast<const struct nk_command_text*>(cmd);
                renderer->drawText(ttfFont, { t->x, t->y }, t->string);
                break;
            }
            case NK_COMMAND_CURVE: { break; } // TODO: no curves?
            case NK_COMMAND_ARC:
            {
                auto a = reinterpret_cast<const struct nk_command_arc*>(cmd);
                renderer->drawArc(a->cx, a->cy, a->r, a->a[0], a->a[1], { a->color.r, a->color.g, a->color.b, a->color.a });
                break;
            }
            case NK_COMMAND_IMAGE:
            {
                auto i = reinterpret_cast<const struct nk_command_image*>(cmd);
                sdl_draw_image(i, i->x, i->y, i->w, i->h);
                break;
            }
            case NK_COMMAND_RECT_MULTI_COLOR:
            case NK_COMMAND_ARC_FILLED:
            default: break;
            }
        }
        nk_clear(&context);
    }        
};
