#pragma once

#define NOMINMAX
#define SDL_MAIN_HANDLED

#include <TLib/Media/Platform/SDL2.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Macros.hpp>
#include <TLib/String.hpp>

#ifdef TLIB_DEBUG
#define ASSERTMSGBOX(expr, title, msg)                                                                                      \
if( !(expr) )                                                                                                               \
{                                                                                                                           \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; \
    std::cerr << "Message: " << msg << std::endl;                                                                           \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, NULL);                                                       \
    abort();                                                                                                                \
}
#else
#define ASSERTMSGBOX(expr, title, msg) ((void)0);
#endif

#define RELASSERTMSGBOX(expr, title, msg)                                                                                   \
if( !(expr) )                                                                                                               \
{                                                                                                                           \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; \
    std::cerr << "Message: " << msg << std::endl;                                                                           \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, NULL);                                                       \
    abort();                                                                                                                \
}

using InputEvent = SDL_Event;

enum class WindowFlags
{
    Fullscreen         = SDL_WINDOW_FULLSCREEN,              /* fullscreen window */
    OpenGL             = SDL_WINDOW_OPENGL,                  /* window usable with GL context */
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
    Vector2i    pos   = { SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED };
    WindowFlags flags = WindowFlags::Resizable | WindowFlags::OpenGL;

    int stencilSize = 1; // Stencil bits per pixel
};

struct Window : NonAssignable
{
    // Read only
    SDL_Window*   window = nullptr;
    SDL_SysWMinfo wm{};
    SDL_GLContext glContext = nullptr;

    Window() = default;
   ~Window() { reset(); }

    void create(const WindowCreateParams& params = WindowCreateParams())
    {
        tlog::info("Creating window...");
        if (created()) { return; }

        SDL_Init(SDL_INIT_EVERYTHING);

        if (params.flags & WindowFlags::OpenGL)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Makes AA available to openGL
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // AA Sample count TODO: make these easier to read & change
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, params.stencilSize);
        }

        window = SDL_CreateWindow(params.title.c_str(),
                                  params.pos.x, params.pos.y,
                                  params.size.x, params.size.y,
                                  static_cast<Uint32>(params.flags));

        if (window == nullptr)
        { tlog::critical("Could not create a window: {}", SDL_GetError()); reset(); }

        SDL_VERSION(&wm.version);
        SDL_GetWindowWMInfo(window, &wm);

        if (params.flags & WindowFlags::OpenGL)
        {
            glContext = SDL_GL_CreateContext(window);
            makeCurrent();
        }

        SDL_AddEventWatch(eventHandler, this);

        tlog::info("Window created");
    }

    void reset()
    {
        if (!created()) { return; }
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    }

    void swap() { SDL_GL_SwapWindow(window); }

    void makeCurrent()
    {
        ASSERT(created());
        if (!glContext)
        {
            tlog::error("Tried to make window '{}' current, when it has no GL context!",
                        SDL_GetWindowTitle(window));
            return;
        }
        SDL_GL_MakeCurrent(window, glContext);
    }

    [[nodiscard]] bool created()
    { return window != nullptr; }

    [[nodiscard]] String getTitle() const
    { return SDL_GetWindowTitle(window); }

    [[nodiscard]] SDL_Surface* getSurface() const noexcept
    { return SDL_GetWindowSurface(window); }

    [[nodiscard]] Vector2i getSize() const noexcept
    {
        Vector2i s;
        SDL_GetWindowSize(window, &s.x, &s.y);
        return s;
    }

    [[nodiscard]] void* getNativeHandle()
    {
        #ifdef OS_WINDOWS
        return wm.info.win.window;
        #elif  OS_MAC
        return info.handle = wm.info.cocoa.window;
        #elif  OS_LINUX
        return (void*)(uintptr_t)wm.info.x11.window;
        #endif
    }

    [[nodiscard]] void* getDisplayType()
    {
        #ifdef OS_WINDOWS 
        return NULL;
        #elif  OS_MAC
        return NULL;
        #elif  OS_LINUX
        return wm.info.x11.display;
        #endif
    }

    [[nodiscard]] Vector2i getFramebufferSize() const noexcept
    {
        int fbw, fbh;
        SDL_GL_GetDrawableSize(window, &fbw, &fbh);
        return { fbw, fbh };
    }

    [[nodiscard]] SDL_GLContext getGLContext() const
    { return glContext; }

    [[nodiscard]] bool hasInputFocus() const
    { return hasFlag(WindowFlags::InputFocus); }

    [[nodiscard]] bool hasMouseFocus() const
    { return hasFlag(WindowFlags::MouseFocus); }

    [[nodiscard]] bool isMinimized() const
    { return hasFlag(WindowFlags::Minimized); }

    [[nodiscard]] bool hasFlag(const WindowFlags flag) const
    { return flag & static_cast<WindowFlags>(SDL_GetWindowFlags(window)); }

    [[nodiscard]] WindowFlags getFlags()
    { return static_cast<WindowFlags>(SDL_GetWindowFlags(window)); }

    void setTitle(const char* title)
    { SDL_SetWindowTitle(window, title); }

    void setTitle(const String& title)
    { SDL_SetWindowTitle(window, title.c_str()); }

    void setSize(const Vector2i& size)
    { SDL_SetWindowSize(window, size.x, size.y); }

    void setSize(int x, int y)
    { SDL_SetWindowSize(window, x, y); }

    void setBordered(bool value)
    { SDL_SetWindowBordered(window, static_cast<SDL_bool>(value)); }

    void setPosition(const Vector2i& pos)
    { setPosition(pos.x, pos.y); }

    void setPosition(int x, int y)
    { SDL_SetWindowPosition(window, x, y); }

    void setResizable(bool value)
    { SDL_SetWindowResizable(window, static_cast<SDL_bool>(value)); }

    // If using with ImGui make sure to enable "ImGuiConfigFlags_NoMouseCursorChange"
    // io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    void showCursor(bool value)
    { SDL_ShowCursor(value); }

    bool isCursorShown() const
    { return SDL_ShowCursor(SDL_QUERY); }

    [[nodiscard]]
    Vector2i getMaxSize() const
    {
        Vector2i ret;
        SDL_GetWindowMaximumSize(window, &ret.x, &ret.y);
        return ret;
    }

    SDL_DisplayMode getDesktopDisplayInfo(int index = 0)
    {
        SDL_DisplayMode info;
        SDL_GetDesktopDisplayMode(index, &info);
        return info;
    }

    [[nodiscard]] String getTitle()
    { return SDL_GetWindowTitle(window); }

    void setFpsMode(bool value)
    {
        if (value)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            showCursor(false);
        }
        else
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            showCursor(true);
        }
    }

    bool getFpsMode() const
    { return SDL_GetRelativeMouseMode(); }

    void toggleFpsMode()
    { setFpsMode(!getFpsMode()); }

    static int eventHandler(void* userdata, SDL_Event* event)
    {
        Window* win = static_cast<Window*>(userdata);
        SDL_Event& e = *event;
       
        if (win->getFpsMode() && e.type == SDL_MOUSEMOTION)
        {
            auto size = win->getSize();
            SDL_WarpMouseInWindow(*win, size.x/2, size.y/2);
        }

        return 0;
    }

    operator SDL_Window*() { return  window; }
    operator SDL_Window&() { return *window; }
};
