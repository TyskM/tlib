#pragma once

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

#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#include "../Macros.hpp"
#include "../Logging.hpp"
#include "../String.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

using InputEvent = SDL_Event;

enum class WindowFlags : int
{
    Fullscreen         = SDL_WINDOW_FULLSCREEN,              /* fullscreen window */
    OpenGL             = SDL_WINDOW_OPENGL,                  /* window usable with OpenGL context */
    Shown              = SDL_WINDOW_SHOWN,                   /* window is visible */
    Hidden             = SDL_WINDOW_HIDDEN,                  /* window is not visible */
    Borderless         = SDL_WINDOW_BORDERLESS,              /* no window decoration */
    Resizable          = SDL_WINDOW_RESIZABLE,               /* window can be resized */
    Minimized          = SDL_WINDOW_MINIMIZED,               /* window is minimized */
    Maximized          = SDL_WINDOW_MAXIMIZED,               /* window is maximized */
    MouseGrabbed       = SDL_WINDOW_MOUSE_GRABBED,           /* window has grabbed mouse input */
    InputFocus         = SDL_WINDOW_INPUT_FOCUS,             /* window has input focus */
    MouseFocus         = SDL_WINDOW_MOUSE_FOCUS,             /* window has mouse focus */
    FullscreenDesktop  = SDL_WINDOW_FULLSCREEN_DESKTOP,         
    Foreign            = SDL_WINDOW_FOREIGN,                 /* window not created by SDL */
    AllowHighDPI       = SDL_WINDOW_ALLOW_HIGHDPI,           /* window should be created in high-DPI mode if supported.
                                                                On macOS NSHighResolutionCapable must be set true in the
                                                                application's Info.plist for this to have any effect. */
    MouseCapture       = SDL_WINDOW_MOUSE_CAPTURE,           /* window has mouse captured (unrelated to MOUSE_GRABBED) */
    AlwaysOnTop        = SDL_WINDOW_ALWAYS_ON_TOP,           /* window should always be above others */
    SkipTaskbar        = SDL_WINDOW_SKIP_TASKBAR,            /* window should not be added to the taskbar */
    Utility            = SDL_WINDOW_UTILITY,                 /* window should be treated as a utility window */
    Tooltip            = SDL_WINDOW_TOOLTIP,                 /* window should be treated as a tooltip */
    PopupMenu          = SDL_WINDOW_POPUP_MENU,              /* window should be treated as a popup menu */
    KeyboardGrabbed    = SDL_WINDOW_KEYBOARD_GRABBED,        /* window has grabbed keyboard input */
    Vulkan             = SDL_WINDOW_VULKAN,                  /* window usable for Vulkan surface */
    Metal              = SDL_WINDOW_METAL,                   /* window usable for Metal view */
};  FLAG_ENUM(WindowFlags);

struct WindowCreateParams
{
    String      title = "Window";
    Vector2i    size  = { 640, 480 };
    Vector2i    pos   = {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED};
    WindowFlags flags = WindowFlags::Resizable;
};

struct Window : NonAssignable
{
    // Read only
    SDL_Window* window = nullptr;
    SDL_SysWMinfo wm;

    Window() = default;
   ~Window() { reset(); }

    void create(const WindowCreateParams& params = WindowCreateParams())
    {
        tlog::info("Creating window...");
        if (created())
        { return; }

        SDL_Init(SDL_INIT_EVERYTHING);

        if (params.flags & WindowFlags::OpenGL)
        { SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); }

        window = SDL_CreateWindow(params.title.c_str(),
                                   params.pos.x, params.pos.y,
                                   params.size.x, params.size.y,
                                   static_cast<Uint32>(params.flags));

        if (window == nullptr)
        { tlog::critical("Could not create a window: {}", SDL_GetError()); reset(); }

        SDL_VERSION(&wm.version);
        SDL_GetWindowWMInfo(window, &wm);

        tlog::info("Window created");
    }

    void reset()
    {
        if (!created()) { return; }
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    }

    bool created() { return window != nullptr; }

    inline SDL_Surface* getSurface() const noexcept
    { return SDL_GetWindowSurface(window); }

    Vector2i getSize() const noexcept
    {
        Vector2i s;
        SDL_GetWindowSize(window, &s.x, &s.y);
        return s;
    }

    void* getNativeHandle()
    {
        #ifdef OS_WINDOWS
        return wm.info.win.window;
        #elif  OS_MAC
        return info.handle = wm.info.cocoa.window;
        #elif  OS_LINUX
        return (void*)(uintptr_t)wm.info.x11.window;
        #endif
    }

    void* getDisplayType()
    {
        #ifdef OS_WINDOWS 
        return NULL;
        #elif  OS_MAC
        return NULL;
        #elif  OS_LINUX
        return wm.info.x11.display;
        #endif
    }

    Vector2i getFramebufferSize() const noexcept
    {
        int fbw, fbh;
        SDL_GL_GetDrawableSize(window, &fbw, &fbh);
        return { fbw, fbh };
    }

    void swap() { SDL_GL_SwapWindow(window); }

    WindowFlags getFlags()
    {
        return static_cast<WindowFlags>(SDL_GetWindowFlags(window));
    }

    operator SDL_Window*() { return  window; }
    operator SDL_Window&() { return *window; }
};
