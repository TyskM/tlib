#pragma once

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "uuid.lib")

#define NOMINMAX
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include "GLHelpers.hpp"
#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#undef main

struct Window : NonAssignable
{
    // Internal vars
    inline static constexpr int subsys = SDL_INIT_TIMER | SDL_INIT_EVENTS;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    Window() { }
    Window(const char* title, int winwidth, int winheight,
           uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
           int winposx = SDL_WINDOWPOS_UNDEFINED, int winposy = SDL_WINDOWPOS_UNDEFINED,
           bool vsync = false)
    {
        create(title, winwidth, winheight, flags, winposx, winposy, vsync);
    }

    void create(const char* title, int winwidth, int winheight,
                uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
                int winposx = SDL_WINDOWPOS_UNDEFINED, int winposy = SDL_WINDOWPOS_UNDEFINED,
                bool vsync = false)
    {
        SDL_Init(subsys);

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        window = SDL_CreateWindow(title, winposx, winposy, winwidth, winheight, flags);

        if (window == nullptr)
        { SDL_Log("Could not create a window: %s", SDL_GetError()); abort(); }

        glContext = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, glContext);
        SDL_GL_SetSwapInterval(vsync);

        if (gl3wInit())
        { fprintf(stderr, "failed to initialize OpenGL\n"); abort(); }
        if (!gl3wIsSupported(3, 3))
        { fprintf(stderr, "OpenGL 3.3 not supported\n"); abort(); }

        printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback( defaultGLCallback, 0 );
    }
    
    SDL_Surface* getSurface() const noexcept
    {
        return SDL_GetWindowSurface(window);
    }

    Vector2i getSize() const noexcept
    {
        Vector2i s;
        SDL_GetWindowSize(window, &s.x, &s.y);
        return s;
    }

    Vector2i getFramebufferSize() const noexcept
    {
        int fbw, fbh;
        SDL_GL_GetDrawableSize(window, &fbw, &fbh);
        return { fbw, fbh };
    }

    operator SDL_Window*() { return window; }
    operator SDL_Window&() { return *window; }

    bool created() { return window != nullptr; }

    void swap() { SDL_GL_SwapWindow(window); }

    ~Window()
    {
        if (created())
        {
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_QuitSubSystem(subsys);
        }
    }
};
