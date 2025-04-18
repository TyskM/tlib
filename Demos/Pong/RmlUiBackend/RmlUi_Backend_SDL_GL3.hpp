/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_GL3.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Profiling.h>

#if SDL_MAJOR_VERSION >= 3
    #include <SDL3_image/SDL_image.h>
#else
    #include <SDL_image.h>
#endif

#if defined RMLUI_PLATFORM_EMSCRIPTEN
    #include <emscripten.h>
#elif SDL_MAJOR_VERSION == 2 && !(SDL_VIDEO_RENDER_OGL)
    #error "Only the OpenGL SDL backend is supported."
#endif

/**
    Custom render interface example for the SDL/GL3 backend.

    Overloads the OpenGL3 render interface to load textures through SDL_image's built-in texture loading functionality.
 */
class RenderInterface_GL3_SDL : public RenderInterface_GL3 {
public:
    RenderInterface_GL3_SDL() {}

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
    {
        Rml::FileInterface* file_interface = Rml::GetFileInterface();
        Rml::FileHandle file_handle = file_interface->Open(source);
        if (!file_handle)
            return {};

        file_interface->Seek(file_handle, 0, SEEK_END);
        const size_t buffer_size = file_interface->Tell(file_handle);
        file_interface->Seek(file_handle, 0, SEEK_SET);

        using Rml::byte;
        Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
        file_interface->Read(buffer.get(), buffer_size, file_handle);
        file_interface->Close(file_handle);

        const size_t i_ext = source.rfind('.');
        Rml::String extension = (i_ext == Rml::String::npos ? Rml::String() : source.substr(i_ext + 1));

#if SDL_MAJOR_VERSION >= 3
        auto CreateSurface = [&]() { return IMG_LoadTyped_IO(SDL_IOFromMem(buffer.get(), int(buffer_size)), 1, extension.c_str()); };
        auto GetSurfaceFormat = [](SDL_Surface* surface) { return surface->format; };
        auto ConvertSurface = [](SDL_Surface* surface, SDL_PixelFormat format) { return SDL_ConvertSurface(surface, format); };
        auto DestroySurface = [](SDL_Surface* surface) { SDL_DestroySurface(surface); };
#else
        auto CreateSurface = [&]() { return IMG_LoadTyped_RW(SDL_RWFromMem(buffer.get(), int(buffer_size)), 1, extension.c_str()); };
        auto GetSurfaceFormat = [](SDL_Surface* surface) { return surface->format->format; };
        auto ConvertSurface = [](SDL_Surface* surface, Uint32 format) { return SDL_ConvertSurfaceFormat(surface, format, 0); };
        auto DestroySurface = [](SDL_Surface* surface) { SDL_FreeSurface(surface); };
#endif

        SDL_Surface* surface = CreateSurface();
        if (!surface)
            return {};

        texture_dimensions = {surface->w, surface->h};

        if (GetSurfaceFormat(surface) != SDL_PIXELFORMAT_RGBA32)
        {
            // Ensure correct format for premultiplied alpha conversion and GenerateTexture below.
            SDL_Surface* converted_surface = ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
            DestroySurface(surface);
            if (!converted_surface)
                return {};

            surface = converted_surface;
        }

        // Convert colors to premultiplied alpha, which is necessary for correct alpha compositing.
        const size_t pixels_byte_size = surface->w * surface->h * 4;
        byte* pixels = static_cast<byte*>(surface->pixels);
        for (size_t i = 0; i < pixels_byte_size; i += 4)
        {
            const byte alpha = pixels[i + 3];
            for (size_t j = 0; j < 3; ++j)
                pixels[i + j] = byte(int(pixels[i + j]) * int(alpha) / 255);
        }

        Rml::TextureHandle texture_handle = RenderInterface_GL3::GenerateTexture({pixels, pixels_byte_size}, texture_dimensions);

        DestroySurface(surface);

        return texture_handle;
    }
};

struct RmlUi_SDL2
{
    SystemInterface_SDL     system_interface;
    RenderInterface_GL3_SDL render_interface;
    SDL_Window*             window = nullptr;

    bool init(SDL_Window* window, int width, int height)
    {
        // Submit click events when focusing the window.
        //SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
        //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // SDL2 implicitly activates text input on window creation. Turn it off for now, it will be activated again e.g. when focusing a text input field.
        SDL_StopTextInput();

        if (!render_interface)
        {
            Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to initialize OpenGL3 render interface");
            return false;
        }

        this->window = window;
        system_interface.SetWindow(window);
        render_interface.SetViewport(width, height);

        return true;
    }

    void processEvents(SDL_Event& ev, Rml::Context* context, KeyDownCallback key_down_callback)
    {
        RMLUI_ASSERT(context);

        auto GetKey = [](const SDL_Event& event) { return event.key.keysym.sym; };
        auto GetDisplayScale = []() { return 1.f; };
        constexpr auto event_key_down = SDL_KEYDOWN;
        constexpr auto event_window_size_changed = SDL_WINDOWEVENT_SIZE_CHANGED;
        int has_event = 0;

        while (has_event)
        {
            switch (ev.type)
            {
            break;
            case event_key_down:
            {
                const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(ev));
                const int key_modifier = RmlSDL::GetKeyModifierState();
                const float native_dp_ratio = GetDisplayScale();

                // See if we have any global shortcuts that take priority over the context.
                if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true))
                    break;
                // Otherwise, hand the event over to the context by calling the input handler as normal.
                if (!RmlSDL::InputEventHandler(context, window, ev))
                    break;
                // The key was not consumed by the context either, try keyboard shortcuts of lower priority.
                if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false))
                    break;
            }
            break;
            case event_window_size_changed:
            {
                Rml::Vector2i dimensions = {ev.window.data1, ev.window.data2};
                render_interface.SetViewport(dimensions.x, dimensions.y);
            }
            break;
            default: break;
            }
        }
    }

    void beginFrame()
    {
        render_interface.BeginFrame();
    }

    void presentFrame()
    {
        render_interface.EndFrame();

        // Optional, used to mark frames during performance profiling.
        RMLUI_FrameMark;
    }
};

