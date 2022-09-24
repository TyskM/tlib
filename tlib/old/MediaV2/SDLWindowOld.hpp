#pragma once

#undef main

#include <string>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/EnumSet.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pointer.h>

#include "Magnum/Magnum.h"
#include "Magnum/Tags.h"
#include "Magnum/Math/Vector4.h"
#include "Magnum/Platform/Platform.h"

#ifdef MAGNUM_TARGET_GL
#include "Magnum/GL/GL.h"
#endif

#ifdef CORRADE_TARGET_WINDOWS
#define SDL_MAIN_HANDLED
#endif

#ifdef CORRADE_TARGET_CLANG_CL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack"
#endif
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_scancode.h>

#ifdef CORRADE_TARGET_IOS
#include <SDL_main.h>
#endif

#ifdef CORRADE_TARGET_WINDOWS_RT
#include <SDL_main.h>
#include <wrl.h>
#endif
#ifdef CORRADE_TARGET_CLANG_CL
#pragma clang diagnostic pop
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
union SDL_Event;
#endif

namespace Magnum { namespace Platform {

    namespace Implementation {
        enum class SimpleSdl2DpiScalingPolicy: UnsignedByte;
    }
    class SDLWindow {
    public:
        struct Arguments { constexpr Arguments(int& argc, char** argv) noexcept: argc{argc}, argv{argv} {}

            int& argc;
            char** argv;
        };

        class Configuration;
#ifdef MAGNUM_TARGET_GL
        class GLConfiguration;
#endif
        class ExitEvent;
        class ViewportEvent;
        class InputEvent;
        class KeyEvent;
        class MouseEvent;
        class MouseMoveEvent;
        class MouseScrollEvent;
        class MultiGestureEvent;
        class TextInputEvent;
        class TextEditingEvent;

#ifdef MAGNUM_TARGET_GL
        explicit SDLWindow(const Arguments& arguments, const Configuration& configuration, const GLConfiguration& glConfiguration);
#endif
        explicit SDLWindow(const Arguments& arguments, const Configuration& configuration);
        explicit SDLWindow(const Arguments& arguments);
        explicit SDLWindow(const Arguments& arguments, NoCreateT);
        SDLWindow(const SDLWindow&) = delete;
        SDLWindow(SDLWindow&&) = delete;
        SDLWindow& operator=(const SDLWindow&) = delete;
        SDLWindow& operator=(SDLWindow&&) = delete;
        int exec();
        bool mainLoopIteration();
        void exit(int exitCode = 0);

        virtual void input(const SDL_Event& e) {  }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_Window* window() { return _window; }
#endif

#if defined(MAGNUM_TARGET_GL) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        SDL_GLContext glContext() { return _glContext; }
#endif

    protected:
        ~SDLWindow();

#ifdef MAGNUM_TARGET_GL
        void create(const Configuration& configuration, const GLConfiguration& glConfiguration);
#endif
        void create(const Configuration& configuration);
        void create();

#ifdef MAGNUM_TARGET_GL
        bool tryCreate(const Configuration& configuration, const GLConfiguration& glConfiguration);
#endif
        bool tryCreate(const Configuration& configuration);

    public:
        Vector2i windowSize() const;

#if !defined(CORRADE_TARGET_EMSCRIPTEN) || defined(DOXYGEN_GENERATING_OUTPUT)
        void setWindowSize(const Vector2i& size);
        void setMinWindowSize(const Vector2i& size);
        void setMaxWindowSize(const Vector2i& size);
#endif

#if defined(MAGNUM_TARGET_GL) || defined(DOXYGEN_GENERATING_OUTPUT)
        Vector2i framebufferSize() const;
#endif
        Vector2 dpiScaling() const { return _dpiScaling; }
        Vector2 dpiScaling(const Configuration& configuration);
        void setWindowTitle(const std::string& title);

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && (SDL_MAJOR_VERSION*1000 + SDL_MINOR_VERSION*100 + SDL_PATCHLEVEL >= 2005 || defined(DOXYGEN_GENERATING_OUTPUT))
        void setWindowIcon(const ImageView2D& image);
#endif

#if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(DOXYGEN_GENERATING_OUTPUT)
        void setContainerCssClass(const std::string& cssClass);
#endif
        void swapBuffers();
        Int swapInterval() const;
        bool setSwapInterval(Int interval);

#ifndef CORRADE_TARGET_EMSCRIPTEN
        void setMinimalLoopPeriod(UnsignedInt milliseconds) {
            _minimalLoopPeriod = milliseconds;
        }
#endif
        void redraw();

    public:
        virtual void viewportEvent(ViewportEvent& event);

#ifdef MAGNUM_BUILD_DEPRECATED
        virtual CORRADE_DEPRECATED("use viewportEvent(ViewportEvent&) instead") void viewportEvent(const Vector2i& size);
#endif
        virtual void drawEvent() = 0;
        virtual void keyPressEvent(KeyEvent& event);
        virtual void keyReleaseEvent(KeyEvent& event);

    public:
        enum class Cursor: UnsignedInt {
            Arrow,
            TextInput,
            Wait,
            Crosshair,
            WaitArrow,
            ResizeNWSE,
            ResizeNESW,
            ResizeWE,
            ResizeNS,
            ResizeAll,
            No,
            Hand,
            Hidden,

#ifndef CORRADE_TARGET_EMSCRIPTEN
                            HiddenLocked
#endif
        };
        void setCursor(Cursor cursor);
        Cursor cursor();

#ifndef CORRADE_TARGET_EMSCRIPTEN
        void warpCursor(const Vector2i& position) {
            SDL_WarpMouseInWindow(_window, position.x(), position.y());
        }
#endif

#ifdef MAGNUM_BUILD_DEPRECATED
        CORRADE_DEPRECATED("use cursor() together with Cursor::HiddenLocked instead") bool isMouseLocked() const { return SDL_GetRelativeMouseMode(); }
        CORRADE_DEPRECATED("use setCursor() together with Cursor::HiddenLocked instead") void setMouseLocked(bool enabled);
#endif

    public:
        virtual void mousePressEvent(MouseEvent& event);
        virtual void mouseReleaseEvent(MouseEvent& event);
        virtual void mouseMoveEvent(MouseMoveEvent& event);
        virtual void mouseScrollEvent(MouseScrollEvent& event);
        virtual void multiGestureEvent(MultiGestureEvent& event);
    public:
        bool isTextInputActive();
        void startTextInput();
        void stopTextInput();
        void setTextInputRect(const Range2Di& rect);

    public:
        virtual void textInputEvent(TextInputEvent& event);
        virtual void textEditingEvent(TextEditingEvent& event);
        virtual void exitEvent(ExitEvent& event);

    protected:
        virtual void tickEvent();

    public:
        virtual void anyEvent(SDL_Event& event);

    public:
        enum class Flag: UnsignedByte;
        typedef Containers::EnumSet<Flag> Flags;
        CORRADE_ENUMSET_FRIEND_OPERATORS(Flags)

#ifndef CORRADE_TARGET_EMSCRIPTEN
            SDL_Cursor* _cursors[14]{};
#else
            Cursor _cursor;
#endif
        bool _verboseLog{};
        Implementation::SimpleSdl2DpiScalingPolicy _commandLineDpiScalingPolicy{};
        Vector2 _commandLineDpiScaling;

        Vector2 _dpiScaling;

#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_Window* _window{};
        UnsignedInt _minimalLoopPeriod;
#else
        SDL_Surface* _surface{};
        Vector2i _lastKnownCanvasSize;
#endif

#ifdef MAGNUM_TARGET_GL
#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_GLContext _glContext{};
#endif
        Containers::Pointer<Platform::GLContext> _context;
#endif

        Flags _flags;

        int _exitCode = 0;
    };

#ifdef MAGNUM_TARGET_GL
    class SDLWindow::GLConfiguration {
    public:
#ifndef CORRADE_TARGET_EMSCRIPTEN
        enum class Flag: int {
#ifndef MAGNUM_TARGET_GLES
            ForwardCompatible = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG,
#endif
            Debug = SDL_GL_CONTEXT_DEBUG_FLAG,
            RobustAccess = SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG,
            ResetIsolation = SDL_GL_CONTEXT_RESET_ISOLATION_FLAG
        };
#ifndef DOXYGEN_GENERATING_OUTPUT
        typedef Containers::EnumSet<Flag, SDL_GL_CONTEXT_DEBUG_FLAG|
            SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG|SDL_GL_CONTEXT_RESET_ISOLATION_FLAG
#ifndef MAGNUM_TARGET_GLES
            |SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
#endif
        > Flags;
#else
        typedef Containers::EnumSet<Flag> Flags;
#endif
#endif

        explicit GLConfiguration();
        ~GLConfiguration();

#ifndef CORRADE_TARGET_EMSCRIPTEN
        Flags flags() const { return _flags; }
        GLConfiguration& setFlags(Flags flags) {
            _flags = flags;
            return *this;
        }
        GLConfiguration& addFlags(Flags flags) {
            _flags |= flags;
            return *this;
        }
        GLConfiguration& clearFlags(Flags flags) {
            _flags &= ~flags;
            return *this;
        }
        GL::Version version() const { return _version; }
#endif
        GLConfiguration& setVersion(GL::Version version) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
            _version = version;
#else
            static_cast<void>(version);
#endif
            return *this;
        }
        Vector4i colorBufferSize() const { return _colorBufferSize; }
        GLConfiguration& setColorBufferSize(const Vector4i& size) {
            _colorBufferSize = size;
            return *this;
        }
        Int depthBufferSize() const { return _depthBufferSize; }
        GLConfiguration& setDepthBufferSize(Int size) {
            _depthBufferSize = size;
            return *this;
        }
        Int stencilBufferSize() const { return _stencilBufferSize; }
        GLConfiguration& setStencilBufferSize(Int size) {
            _stencilBufferSize = size;
            return *this;
        }
        Int sampleCount() const { return _sampleCount; }
        GLConfiguration& setSampleCount(Int count) {
            _sampleCount = count;
            return *this;
        }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        bool isSrgbCapable() const { return _srgbCapable; }
        GLConfiguration& setSrgbCapable(bool enabled) {
            _srgbCapable = enabled;
            return *this;
        }

#ifdef MAGNUM_BUILD_DEPRECATED
        CORRADE_DEPRECATED("use isSrgbCapable() instead") bool isSRGBCapable() const { return isSrgbCapable(); }
        CORRADE_DEPRECATED("use setSrgbCapable() instead") GLConfiguration& setSRGBCapable(bool enabled) {
            return setSrgbCapable(enabled);
        }
#endif
#endif

    public:
        Vector4i _colorBufferSize;
        Int _depthBufferSize, _stencilBufferSize;
        Int _sampleCount;
#ifndef CORRADE_TARGET_EMSCRIPTEN
        GL::Version _version;
        Flags _flags;
        bool _srgbCapable;
#endif
    };

#ifndef CORRADE_TARGET_EMSCRIPTEN
    CORRADE_ENUMSET_OPERATORS(SDLWindow::GLConfiguration::Flags)
#endif
#endif

        namespace Implementation {
        enum class SimpleSdl2DpiScalingPolicy: UnsignedByte {

#ifdef CORRADE_TARGET_APPLE
            Framebuffer = 1,
#endif

#ifndef CORRADE_TARGET_APPLE
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
            Virtual = 2,
#endif

            Physical = 3,
#endif

            Default
#ifdef CORRADE_TARGET_APPLE
            = Framebuffer
#elif !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
            = Virtual
#else
            = Physical
#endif
        };
    }
    class SDLWindow::Configuration {
    public:
        enum class WindowFlag: Uint32 {
            Resizable = SDL_WINDOW_RESIZABLE,

#ifndef CORRADE_TARGET_EMSCRIPTEN
            Fullscreen = SDL_WINDOW_FULLSCREEN,
            FullscreenDesktop = SDL_WINDOW_FULLSCREEN_DESKTOP,
            Borderless = SDL_WINDOW_BORDERLESS,
#endif

#ifdef MAGNUM_BUILD_DEPRECATED
            AllowHighDpi CORRADE_DEPRECATED_ENUM("has no effect, passed implicitly on platforms that need it") = 0,
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
            Hidden = SDL_WINDOW_HIDDEN,
            Maximized = SDL_WINDOW_MAXIMIZED,
            Minimized = SDL_WINDOW_MINIMIZED,
            MouseLocked = SDL_WINDOW_INPUT_GRABBED,

#if SDL_MAJOR_VERSION*1000 + SDL_MINOR_VERSION*100 + SDL_PATCHLEVEL >= 2005 || defined(DOXYGEN_GENERATING_OUTPUT)
            AlwaysOnTop = SDL_WINDOW_ALWAYS_ON_TOP,
            SkipTaskbar = SDL_WINDOW_SKIP_TASKBAR,
            Utility = SDL_WINDOW_UTILITY,
            Tooltip = SDL_WINDOW_TOOLTIP,
            PopupMenu = SDL_WINDOW_POPUP_MENU,
#endif
#endif
            Contextless = 1u << 31,
                                    OpenGL = SDL_WINDOW_OPENGL,

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && (SDL_MAJOR_VERSION*1000 + SDL_MINOR_VERSION*100 + SDL_PATCHLEVEL >= 2006 || defined(DOXYGEN_GENERATING_OUTPUT))
                                    Vulkan = SDL_WINDOW_VULKAN
#endif
        };
#ifndef DOXYGEN_GENERATING_OUTPUT
        typedef Containers::EnumSet<WindowFlag, SDL_WINDOW_RESIZABLE|
#ifndef CORRADE_TARGET_EMSCRIPTEN
            SDL_WINDOW_FULLSCREEN|SDL_WINDOW_BORDERLESS|SDL_WINDOW_HIDDEN|
            SDL_WINDOW_MAXIMIZED|SDL_WINDOW_MINIMIZED|SDL_WINDOW_INPUT_GRABBED|
#endif
            Uint32(WindowFlag::Contextless)|SDL_WINDOW_OPENGL
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && SDL_MAJOR_VERSION*1000 + SDL_MINOR_VERSION*100 + SDL_PATCHLEVEL >= 2006
            |SDL_WINDOW_VULKAN
#endif
        > WindowFlags;
#else
        typedef Containers::EnumSet<WindowFlag> WindowFlags;
#endif
#ifdef DOXYGEN_GENERATING_OUTPUT
        enum class DpiScalingPolicy: UnsignedByte {
            Framebuffer,
            Virtual,
            Physical,
            Default
        };
#else
        typedef Implementation::SimpleSdl2DpiScalingPolicy DpiScalingPolicy;
#endif Configuration();
        ~Configuration();

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_IOS)
        std::string title() const { return _title; }
#endif
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_IOS)
        Configuration& setTitle(std::string title) {
            _title = std::move(title);
            return *this;
        }
#else
        template<class T> Configuration& setTitle(const T&) { return *this; }
#endif
        Vector2i size() const { return _size; }
        DpiScalingPolicy dpiScalingPolicy() const { return _dpiScalingPolicy; }
        Vector2 dpiScaling() const { return _dpiScaling; }
        Configuration& setSize(const Vector2i& size, DpiScalingPolicy dpiScalingPolicy = DpiScalingPolicy::Default) {
            _size = size;
            _dpiScalingPolicy = dpiScalingPolicy;
            return *this;
        }
        Configuration& setSize(const Vector2i& size, const Vector2& dpiScaling) {
            _size = size;
            _dpiScaling = dpiScaling;
            return *this;
        }
        WindowFlags windowFlags() const { return _windowFlags; }
        Configuration& setWindowFlags(WindowFlags flags) {
            _windowFlags = flags;
            return *this;
        }
        Configuration& addWindowFlags(WindowFlags flags) {
            _windowFlags |= flags;
            return *this;
        }
        Configuration& clearWindowFlags(WindowFlags flags) {
            _windowFlags &= ~flags;
            return *this;
        }

    public:
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_IOS)
        std::string _title;
#endif
        Vector2i _size;
        DpiScalingPolicy _dpiScalingPolicy;
        WindowFlags _windowFlags;
        Vector2 _dpiScaling;
    };
    class SDLWindow::ExitEvent {
    public:
        ExitEvent(const ExitEvent&) = delete;
        ExitEvent(ExitEvent&&) = delete;
        ExitEvent& operator=(const ExitEvent&) = delete;
        ExitEvent& operator=(ExitEvent&&) = delete;
        bool isAccepted() const { return _accepted; }
        void setAccepted(bool accepted = true) { _accepted = accepted; }
        const SDL_Event& event() const { return _event; }

    public:
        friend SDLWindow;

        explicit ExitEvent(const SDL_Event& event): _event(event), _accepted(false) {}

        const SDL_Event& _event;
        bool _accepted;
    };
    class SDLWindow::ViewportEvent {
    public:
        ViewportEvent(const ViewportEvent&) = delete;
        ViewportEvent(ViewportEvent&&) = delete;
        ViewportEvent& operator=(const ViewportEvent&) = delete;
        ViewportEvent& operator=(ViewportEvent&&) = delete;
        Vector2i windowSize() const { return _windowSize; }

#if defined(MAGNUM_TARGET_GL) || defined(DOXYGEN_GENERATING_OUTPUT)
        Vector2i framebufferSize() const { return _framebufferSize; }
#endif
        Vector2 dpiScaling() const { return _dpiScaling; }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        const SDL_Event& event() const { return _event; }
#endif

    public:
        friend SDLWindow;

        explicit ViewportEvent(
#ifndef CORRADE_TARGET_EMSCRIPTEN
            const SDL_Event& event,
#endif
            const Vector2i& windowSize,
#ifdef MAGNUM_TARGET_GL
            const Vector2i& framebufferSize,
#endif
            const Vector2& dpiScaling):
#ifndef CORRADE_TARGET_EMSCRIPTEN
            _event(event),
#endif
            _windowSize{windowSize},
#ifdef MAGNUM_TARGET_GL
            _framebufferSize{framebufferSize},
#endif
            _dpiScaling{dpiScaling} {}

#ifndef CORRADE_TARGET_EMSCRIPTEN
        const SDL_Event& _event;
#endif
        const Vector2i _windowSize;
#ifdef MAGNUM_TARGET_GL
        const Vector2i _framebufferSize;
#endif
        const Vector2 _dpiScaling;
    };
    class SDLWindow::InputEvent {
    public:
        enum class Modifier: Uint16 {
            Shift = KMOD_SHIFT,
            Ctrl = KMOD_CTRL,
            Alt = KMOD_ALT,
            Super = KMOD_GUI,
            AltGr = KMOD_MODE,

            CapsLock = KMOD_CAPS,
            NumLock = KMOD_NUM
        };
        typedef Containers::EnumSet<Modifier> Modifiers;
        InputEvent(const InputEvent&) = delete;
        InputEvent(InputEvent&&) = delete;
        InputEvent& operator=(const InputEvent&) = delete;
        InputEvent& operator=(InputEvent&&) = delete;
        bool isAccepted() const { return _accepted; }
        void setAccepted(bool accepted = true) { _accepted = accepted; }
        const SDL_Event& event() const { return _event; }

#ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
        explicit InputEvent(const SDL_Event& event): _event(event), _accepted(false) {}

        ~InputEvent() = default;
#endif

    public:
        const SDL_Event& _event;
        bool _accepted;
    };
    class SDLWindow::KeyEvent: public SDLWindow::InputEvent {
    public:
        enum class Key: SDL_Keycode {
            Unknown = SDLK_UNKNOWN,
                                        LeftShift = SDLK_LSHIFT,
                                        RightShift = SDLK_RSHIFT,
                                        LeftCtrl = SDLK_LCTRL,
                                        RightCtrl = SDLK_RCTRL,
                                        LeftAlt = SDLK_LALT,
                                        RightAlt = SDLK_RALT,
                                        LeftSuper = SDLK_LGUI,
                                        RightSuper = SDLK_RGUI,
                                        AltGr = SDLK_MODE,

                                        Enter = SDLK_RETURN,
                                        Esc = SDLK_ESCAPE,

                                        Up = SDLK_UP,
                                        Down = SDLK_DOWN,
                                        Left = SDLK_LEFT,
                                        Right = SDLK_RIGHT,
                                        Home = SDLK_HOME,
                                        End = SDLK_END,
                                        PageUp = SDLK_PAGEUP,
                                        PageDown = SDLK_PAGEDOWN,
                                        Backspace = SDLK_BACKSPACE,
                                        Insert = SDLK_INSERT,
                                        Delete = SDLK_DELETE,

                                        F1 = SDLK_F1,
                                        F2 = SDLK_F2,
                                        F3 = SDLK_F3,
                                        F4 = SDLK_F4,
                                        F5 = SDLK_F5,
                                        F6 = SDLK_F6,
                                        F7 = SDLK_F7,
                                        F8 = SDLK_F8,
                                        F9 = SDLK_F9,
                                        F10 = SDLK_F10,
                                        F11 = SDLK_F11,
                                        F12 = SDLK_F12,

                                        Space = SDLK_SPACE,
                                        Tab = SDLK_TAB,
                                                                    Quote = SDLK_QUOTE,

                                                                    Comma = SDLK_COMMA,
                                                                    Period = SDLK_PERIOD,
                                                                    Minus = SDLK_MINUS,
                                                                    Plus = SDLK_PLUS,
                                                                    Slash = SDLK_SLASH,
                                                                    Percent = SDLK_PERCENT,
                                                                    Semicolon = SDLK_SEMICOLON,
                                                                    Equal = SDLK_EQUALS,
                                                                                                LeftBracket = SDLK_LEFTBRACKET,
                                                                                                RightBracket = SDLK_RIGHTBRACKET,
                                                                                                Backslash = SDLK_BACKSLASH,
                                                                                                Backquote = SDLK_BACKQUOTE,

                                                                                                Zero = SDLK_0,
                                                                                                One = SDLK_1,
                                                                                                Two = SDLK_2,
                                                                                                Three = SDLK_3,
                                                                                                Four = SDLK_4,
                                                                                                Five = SDLK_5,
                                                                                                Six = SDLK_6,
                                                                                                Seven = SDLK_7,
                                                                                                Eight = SDLK_8,
                                                                                                Nine = SDLK_9,

                                                                                                A = SDLK_a,
                                                                                                B = SDLK_b,
                                                                                                C = SDLK_c,
                                                                                                D = SDLK_d,
                                                                                                E = SDLK_e,
                                                                                                F = SDLK_f,
                                                                                                G = SDLK_g,
                                                                                                H = SDLK_h,
                                                                                                I = SDLK_i,
                                                                                                J = SDLK_j,
                                                                                                K = SDLK_k,
                                                                                                L = SDLK_l,
                                                                                                M = SDLK_m,
                                                                                                N = SDLK_n,
                                                                                                O = SDLK_o,
                                                                                                P = SDLK_p,
                                                                                                Q = SDLK_q,
                                                                                                R = SDLK_r,
                                                                                                S = SDLK_s,
                                                                                                T = SDLK_t,
                                                                                                U = SDLK_u,
                                                                                                V = SDLK_v,
                                                                                                W = SDLK_w,
                                                                                                X = SDLK_x,
                                                                                                Y = SDLK_y,
                                                                                                Z = SDLK_z,

                                                                                                NumZero = SDLK_KP_0,
                                                                                                NumOne = SDLK_KP_1,
                                                                                                NumTwo = SDLK_KP_2,
                                                                                                NumThree = SDLK_KP_3,
                                                                                                NumFour = SDLK_KP_4,
                                                                                                NumFive = SDLK_KP_5,
                                                                                                NumSix = SDLK_KP_6,
                                                                                                NumSeven = SDLK_KP_7,
                                                                                                NumEight = SDLK_KP_8,
                                                                                                NumNine = SDLK_KP_9,
                                                                                                NumDecimal = SDLK_KP_DECIMAL,
                                                                                                NumDivide = SDLK_KP_DIVIDE,
                                                                                                NumMultiply = SDLK_KP_MULTIPLY,
                                                                                                NumSubtract = SDLK_KP_MINUS,
                                                                                                NumAdd = SDLK_KP_PLUS,
                                                                                                NumEnter = SDLK_KP_ENTER,
                                                                                                NumEqual = SDLK_KP_EQUALS
        };
        static std::string keyName(Key key);
        Key key() const { return _key; }
        std::string keyName() const;
        Modifiers modifiers() const { return _modifiers; }
        bool isRepeated() const { return _repeated; }

    public:
        friend SDLWindow;

        explicit KeyEvent(const SDL_Event& event, Key key, Modifiers modifiers, bool repeated): InputEvent{event}, _key{key}, _modifiers{modifiers}, _repeated{repeated} {}

        const Key _key;
        const Modifiers _modifiers;
        const bool _repeated;
    };
    class SDLWindow::MouseEvent: public SDLWindow::InputEvent {
    public:
        enum class Button: Uint8 {
            Left = SDL_BUTTON_LEFT,
            Middle = SDL_BUTTON_MIDDLE,
            Right = SDL_BUTTON_RIGHT,
                                            X1 = SDL_BUTTON_X1,
                                            X2 = SDL_BUTTON_X2,
        };
        Button button() const { return _button; }
        Vector2i position() const { return _position; }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        Int clickCount() const { return _clickCount; }
#endif
        Modifiers modifiers();

    public:
        friend SDLWindow;

        explicit MouseEvent(const SDL_Event& event, Button button, const Vector2i& position
#ifndef CORRADE_TARGET_EMSCRIPTEN
            , Int clickCount
#endif
        ): InputEvent{event}, _button{button}, _position{position}
#ifndef CORRADE_TARGET_EMSCRIPTEN
            , _clickCount{clickCount}
#endif
        {}

        const Button _button;
        const Vector2i _position;
#ifndef CORRADE_TARGET_EMSCRIPTEN
        const Int _clickCount;
#endif
        Containers::Optional<Modifiers> _modifiers;
    };
    class SDLWindow::MouseMoveEvent: public SDLWindow::InputEvent {
    public:
        enum class Button: Uint32 {
            Left = SDL_BUTTON_LMASK,
            Middle = SDL_BUTTON_MMASK,
            Right = SDL_BUTTON_RMASK,
                                            X1 = SDL_BUTTON_X1MASK,
                                            X2 = SDL_BUTTON_X2MASK
        };
        typedef Containers::EnumSet<Button> Buttons;
        Vector2i position() const { return _position; }
        Vector2i relativePosition() const { return _relativePosition; }
        Buttons buttons() const { return _buttons; }
        Modifiers modifiers();

    public:
        friend SDLWindow;

        explicit MouseMoveEvent(const SDL_Event& event, const Vector2i& position, const Vector2i& relativePosition, Buttons buttons): InputEvent{event}, _position{position}, _relativePosition{relativePosition}, _buttons{buttons} {}

        const Vector2i _position, _relativePosition;
        const Buttons _buttons;
        Containers::Optional<Modifiers> _modifiers;
    };
    class SDLWindow::MouseScrollEvent: public SDLWindow::InputEvent {
    public:
        Vector2 offset() const { return _offset; }
        Vector2i position();
        Modifiers modifiers();

    public:
        friend SDLWindow;

        explicit MouseScrollEvent(const SDL_Event& event, const Vector2& offset): InputEvent{event}, _offset{offset} {}

        const Vector2 _offset;
        Containers::Optional<Vector2i> _position;
        Containers::Optional<Modifiers> _modifiers;
    };
    class SDLWindow::MultiGestureEvent {
    public:
        MultiGestureEvent(const MultiGestureEvent&) = delete;
        MultiGestureEvent(MultiGestureEvent&&) = delete;
        MultiGestureEvent& operator=(const MultiGestureEvent&) = delete;
        MultiGestureEvent& operator=(MultiGestureEvent&&) = delete;
        bool isAccepted() const { return _accepted; }
        void setAccepted(bool accepted = true) { _accepted = accepted; }
        Vector2 center() const { return _center; }
        Float relativeRotation() const { return _relativeRotation; }
        Float relativeDistance() const { return _relativeDistance; }
        Int fingerCount() const { return _fingerCount; }
        const SDL_Event& event() const { return _event; }

    public:
        friend SDLWindow;

        explicit MultiGestureEvent(const SDL_Event& event, const Vector2& center, Float relativeRotation, Float relativeDistance, Int fingerCount): _event(event), _center{center}, _relativeRotation{relativeRotation}, _relativeDistance{relativeDistance}, _fingerCount{fingerCount}, _accepted{false} {}

        const SDL_Event& _event;
        const Vector2 _center;
        const Float _relativeRotation;
        const Float _relativeDistance;
        const Int _fingerCount;
        bool _accepted;
    };
    class SDLWindow::TextInputEvent {
    public:
        TextInputEvent(const TextInputEvent&) = delete;
        TextInputEvent(TextInputEvent&&) = delete;
        TextInputEvent& operator=(const TextInputEvent&) = delete;
        TextInputEvent& operator=(TextInputEvent&&) = delete;
        bool isAccepted() const { return _accepted; }
        void setAccepted(bool accepted = true) { _accepted = accepted; }
        Containers::ArrayView<const char> text() const { return _text; }
        const SDL_Event& event() const { return _event; }

    public:
        friend SDLWindow;

        explicit TextInputEvent(const SDL_Event& event, Containers::ArrayView<const char> text): _event(event), _text{text}, _accepted{false} {}

        const SDL_Event& _event;
        const Containers::ArrayView<const char> _text;
        bool _accepted;
    };
    class SDLWindow::TextEditingEvent {
    public:
        TextEditingEvent(const TextEditingEvent&) = delete;
        TextEditingEvent(TextEditingEvent&&) = delete;
        TextEditingEvent& operator=(const TextEditingEvent&) = delete;
        TextEditingEvent& operator=(TextEditingEvent&&) = delete;
        bool isAccepted() const { return _accepted; }
        void setAccepted(bool accepted = true) { _accepted = accepted; }
        Containers::ArrayView<const char> text() const { return _text; }
        Int start() const { return _start; }
        Int length() const { return _length; }
        const SDL_Event& event() const { return _event; }

    public:
        friend SDLWindow;

        explicit TextEditingEvent(const SDL_Event& event, Containers::ArrayView<const char> text, Int start, Int length): _event(event), _text{text}, _start{start}, _length{length}, _accepted{false} {}

        const SDL_Event& _event;
        const Containers::ArrayView<const char> _text;
        const Int _start;
        const Int _length;
        bool _accepted;
    };
#ifndef CORRADE_TARGET_WINDOWS_RT
#define MAGNUM_SDLWindow_MAIN(className)                              \
    int main(int argc, char** argv) {                                       \
        className app({argc, argv});                                        \
        return app.exec();                                                  \
    }
#else
#define MAGNUM_SDLWindow_MAIN(className)                              \
    int main(int argc, char** argv) {                                       \
        className app({argc, argv});                                        \
        return app.exec();                                                  \
    }                                                                       \
    __pragma(warning(push))                                                 \
    __pragma(warning(disable: 4447))                                        \
    int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {                \
        if(FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)))  \
            return 1;                                                       \
        return SDL_WinRTRunApp(main, nullptr);                              \
    }                                                                       \
    __pragma(warning(pop))
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef MAGNUM_APPLICATION_MAIN
    typedef SDLWindow Application;
    typedef BasicScreen<SDLWindow> Screen;
    typedef BasicScreenedApplication<SDLWindow> ScreenedApplication;
#define MAGNUM_APPLICATION_MAIN(className) MAGNUM_SDLWindow_MAIN(className)
#else
#undef MAGNUM_APPLICATION_MAIN
#endif
#endif

    CORRADE_ENUMSET_OPERATORS(SDLWindow::Configuration::WindowFlags)
        CORRADE_ENUMSET_OPERATORS(SDLWindow::InputEvent::Modifiers)
        CORRADE_ENUMSET_OPERATORS(SDLWindow::MouseMoveEvent::Buttons)

}}
#pragma region DPIScaling
#include <Corrade/Utility/Utility.h>

#include "Magnum/Magnum.h"

namespace Magnum { namespace Platform { namespace Implementation {

    Utility::Arguments windowScalingArguments();

#ifdef _MAGNUM_PLATFORM_USE_X11
    Float x11DpiScaling();
#endif

#ifdef CORRADE_TARGET_EMSCRIPTEN
    Float emscriptenDpiScaling();
#endif

#ifdef CORRADE_TARGET_APPLE
    bool isAppleBundleHiDpiEnabled();
#endif

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    bool isWindowsAppDpiAware();
#endif

}}}

#include <Corrade/Utility/Arguments.h>

#include "Magnum/Magnum.h"

#ifdef _MAGNUM_PLATFORM_USE_X11
#include <cstring>
#include <dlfcn.h>
#include <X11/Xresource.h>
#include <Corrade/Containers/ScopeGuard.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Debug.h>
#undef None
#endif

#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#define NOMINMAX
#include <windows.h>
#ifdef __has_include
#if __has_include(<shellscalingapi.h>)
#include <shellscalingapi.h>
#endif
#endif
#include <Corrade/Utility/Assert.h>
#endif

namespace Magnum { namespace Platform { namespace Implementation {

    Utility::Arguments windowScalingArguments() {
        Utility::Arguments args{"magnum"};
        args.addOption("dpi-scaling", "default")
            .setFromEnvironment("dpi-scaling")
#ifdef CORRADE_TARGET_APPLE
            .setHelp("dpi-scaling", "\n      window DPI scaling", "default|framebuffer|<d>|\"<h> <v>\"")
#elif !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
            .setHelp("dpi-scaling", "\n      window DPI scaling", "default|virtual|physical|<d>|\"<h> <v>\"")
#else
            .setHelp("dpi-scaling", "\n      window DPI scaling", "default|physical|<d>|\"<h> <v>\"")
#endif
            ;
        return args;
    }

#ifdef _MAGNUM_PLATFORM_USE_X11
    Float x11DpiScaling() {
        void* xlib = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
        Containers::ScopeGuard closeXlib{xlib, dlclose};
#ifdef __GNUC__
        __extension__
#endif
            auto xOpenDisplay = reinterpret_cast<Display*(*)(char*)>(dlsym(xlib, "XOpenDisplay"));
#ifdef __GNUC__
        __extension__
#endif
            auto xCloseDisplay = reinterpret_cast<int(*)(Display*)>(dlsym(xlib, "XCloseDisplay"));
#ifdef __GNUC__
        __extension__
#endif
            auto xResourceManagerString = reinterpret_cast<char*(*)(Display*)>(dlsym(xlib, "XResourceManagerString"));
#ifdef __GNUC__
        __extension__
#endif
            auto xrmGetStringDatabase = reinterpret_cast<XrmDatabase(*)(const char*)>(dlsym(xlib, "XrmGetStringDatabase"));
#ifdef __GNUC__
        __extension__
#endif
            auto xrmGetResource = reinterpret_cast<int(*)(XrmDatabase, const char*, const char*, char**, XrmValue*)>(dlsym(xlib, "XrmGetResource"));
#ifdef __GNUC__
        __extension__
#endif
            auto xrmDestroyDatabase = reinterpret_cast<void(*)(XrmDatabase)>(dlsym(xlib, "XrmDestroyDatabase"));
        if(!xOpenDisplay || !xCloseDisplay || !xResourceManagerString || !xrmGetStringDatabase || !xrmGetResource || !xrmDestroyDatabase) {
            Warning{} << "Platform: can't load X11 symbols for getting virtual DPI scaling, falling back to physical DPI";
            return {};
        }

        Display* display = xOpenDisplay(nullptr);
        Containers::ScopeGuard closeDisplay{display, xCloseDisplay};

        const char* rms = xResourceManagerString(display);
        if(rms) {
            XrmDatabase db = xrmGetStringDatabase(rms);
            CORRADE_INTERNAL_ASSERT(db);
            Containers::ScopeGuard closeDb{db, xrmDestroyDatabase};

            XrmValue value;
            char* type{};
            if(xrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
                if(type && strcmp(type, "String") == 0) {
                    const float scaling = std::stof(value.addr)/96.0f;
                    CORRADE_INTERNAL_ASSERT(scaling);
                    return scaling;
                }
            }
        }

        Warning{} << "Platform: can't get Xft.dpi property for virtual DPI scaling, falling back to physical DPI";
        return {};
    }
#endif

#ifdef CORRADE_TARGET_EMSCRIPTEN
    Float emscriptenDpiScaling() {
        return Float(emscripten_get_device_pixel_ratio());
    }
#endif

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
    bool isWindowsAppDpiAware() {

#ifdef DPI_ENUMS_DECLARED
        HMODULE const shcore = GetModuleHandleA("Shcore.dll");
        if(shcore) {
            auto* const getProcessDpiAwareness = reinterpret_cast<HRESULT(WINAPI *)(HANDLE, PROCESS_DPI_AWARENESS*)>(GetProcAddress(shcore, "GetProcessDpiAwareness"));
            PROCESS_DPI_AWARENESS result{};
            return getProcessDpiAwareness && getProcessDpiAwareness(nullptr, &result) == S_OK && result != PROCESS_DPI_UNAWARE;
        }
#endif
        HMODULE const user32 = GetModuleHandleA("User32.dll");
        CORRADE_INTERNAL_ASSERT(user32);
        auto const isProcessDPIAware = reinterpret_cast<BOOL(WINAPI *)()>(GetProcAddress(user32, "IsProcessDPIAware"));
        CORRADE_INTERNAL_ASSERT(isProcessDPIAware);
        return isProcessDPIAware();
    }
#endif

}}}
#pragma endregion

#include <cstring>
#ifdef CORRADE_TARGET_CLANG_CL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack"
#endif
#include <SDL2/SDL.h>
#ifdef CORRADE_TARGET_CLANG_CL
#pragma clang diagnostic pop
#endif
#ifndef CORRADE_TARGET_EMSCRIPTEN
#include <tuple>
#else
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif
#include <Corrade/Utility/Arguments.h>

#include "Magnum/ImageView.h"
#include "Magnum/PixelFormat.h"
#include "Magnum/Math/ConfigurationValue.h"
#include "Magnum/Math/FunctionsBatch.h"
#include "Magnum/Math/Range.h"
#include "Magnum/Platform/ScreenedApplication.hpp"

#ifdef MAGNUM_TARGET_GL
#include "Magnum/GL/Version.h"
#include "Magnum/Platform/GLContext.h"
#endif

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#endif

namespace Magnum { namespace Platform {

    namespace {
        SDLWindow::InputEvent::Modifiers fixedModifiers(Uint16 mod) {
            SDLWindow::InputEvent::Modifiers modifiers(static_cast<SDLWindow::InputEvent::Modifier>(mod));
            if(modifiers & SDLWindow::InputEvent::Modifier::Shift) modifiers |= SDLWindow::InputEvent::Modifier::Shift;
            if(modifiers & SDLWindow::InputEvent::Modifier::Ctrl) modifiers |= SDLWindow::InputEvent::Modifier::Ctrl;
            if(modifiers & SDLWindow::InputEvent::Modifier::Alt) modifiers |= SDLWindow::InputEvent::Modifier::Alt;
            if(modifiers & SDLWindow::InputEvent::Modifier::Super) modifiers |= SDLWindow::InputEvent::Modifier::Alt;
            return modifiers;
        }

    }

    enum class SDLWindow::Flag: UnsignedByte {
        Redraw = 1 << 0,
        VSyncEnabled = 1 << 1,
        NoTickEvent = 1 << 2,
        NoAnyEvent = 1 << 3,
        Exit = 1 << 4,
#ifdef CORRADE_TARGET_EMSCRIPTEN
        TextInputActive = 1 << 5,
        Resizable = 1 << 6,
#endif
#ifdef CORRADE_TARGET_APPLE
        HiDpiWarningPrinted = 1 << 7
#endif
    };

    SDLWindow::SDLWindow(const Arguments& arguments): SDLWindow{arguments, Configuration{}} {}

    SDLWindow::SDLWindow(const Arguments& arguments, const Configuration& configuration): SDLWindow{arguments, NoCreate} {
        create(configuration);
    }

#ifdef MAGNUM_TARGET_GL
    SDLWindow::SDLWindow(const Arguments& arguments, const Configuration& configuration, const GLConfiguration& glConfiguration): SDLWindow{arguments, NoCreate} {
        create(configuration, glConfiguration);
    }
#endif

    SDLWindow::SDLWindow(const Arguments& arguments, NoCreateT):
#ifndef CORRADE_TARGET_EMSCRIPTEN
        _minimalLoopPeriod{0},
#endif
        _flags{Flag::Redraw}
    {
        Utility::Arguments args{Implementation::windowScalingArguments()};
#ifdef MAGNUM_TARGET_GL
        _context.reset(new GLContext{NoCreate, args, arguments.argc, arguments.argv});
#else
        args.addOption("log", "default").setHelp("log", "console logging", "default|quiet|verbose")
            .setFromEnvironment("log")
            .parse(arguments.argc, arguments.argv);
#endif
#ifdef SDL_HINT_NO_SIGNAL_HANDLERS
        SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif
#if !defined(MAGNUM_TARGET_DESKTOP_GLES) && defined(SDL_HINT_OPENGL_ES_DRIVER)
        SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
#endif
#ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            Error() << "Cannot initialize SDL:" << SDL_GetError();
            std::exit(1);
        }
        if(args.value("log") == "verbose") _verboseLog = true;
        const std::string dpiScaling = args.value("dpi-scaling");
        if(dpiScaling == "default")
            _commandLineDpiScalingPolicy = Implementation::SimpleSdl2DpiScalingPolicy::Default;
#ifdef CORRADE_TARGET_APPLE
        else if(dpiScaling == "framebuffer")
            _commandLineDpiScalingPolicy = Implementation::SimpleSdl2DpiScalingPolicy::Framebuffer;
#endif
#ifndef CORRADE_TARGET_APPLE
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        else if(dpiScaling == "virtual")
            _commandLineDpiScalingPolicy = Implementation::SimpleSdl2DpiScalingPolicy::Virtual;
#endif
        else if(dpiScaling == "physical")
            _commandLineDpiScalingPolicy = Implementation::SimpleSdl2DpiScalingPolicy::Physical;
#endif
        else if(dpiScaling.find_first_of(" \t\n") != std::string::npos)
            _commandLineDpiScaling = args.value<Vector2>("dpi-scaling");
        else
            _commandLineDpiScaling = Vector2{args.value<Float>("dpi-scaling")};
    }

    void SDLWindow::create() {
        create(Configuration{});
    }

    void SDLWindow::create(const Configuration& configuration) {
        if(!tryCreate(configuration)) std::exit(1);
    }

#ifdef MAGNUM_TARGET_GL
    void SDLWindow::create(const Configuration& configuration, const GLConfiguration& glConfiguration) {
        if(!tryCreate(configuration, glConfiguration)) std::exit(1);
    }
#endif

    Vector2 SDLWindow::dpiScaling(const Configuration& configuration) {
        std::ostream* verbose = _verboseLog ? Debug::output() : nullptr;
#ifdef CORRADE_TARGET_APPLE
        if(!Implementation::isAppleBundleHiDpiEnabled() && !(_flags & Flag::HiDpiWarningPrinted)) {
            Warning{} << "Platform::SDLWindow: warning: the executable is not a HiDPI-enabled app bundle";
            _flags |= Flag::HiDpiWarningPrinted;
        }
#elif defined(CORRADE_TARGET_WINDOWS)
#endif
        Implementation::SimpleSdl2DpiScalingPolicy dpiScalingPolicy{};
        if(!_commandLineDpiScaling.isZero()) {
            Debug{verbose} << "Platform::SDLWindow: user-defined DPI scaling" << _commandLineDpiScaling.x();
            return _commandLineDpiScaling;
        } else if(_commandLineDpiScalingPolicy != Implementation::SimpleSdl2DpiScalingPolicy::Default) {
            dpiScalingPolicy = _commandLineDpiScalingPolicy;
        } else if(!configuration.dpiScaling().isZero()) {
            Debug{verbose} << "Platform::SDLWindow: app-defined DPI scaling" << _commandLineDpiScaling.x();
            return configuration.dpiScaling();
        } else {
            dpiScalingPolicy = configuration.dpiScalingPolicy();
        }
#ifdef CORRADE_TARGET_APPLE
        return Vector2{1.0f};
#else
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        if(dpiScalingPolicy == Implementation::SimpleSdl2DpiScalingPolicy::Virtual) {
#ifdef _MAGNUM_PLATFORM_USE_X11
            const Vector2 dpiScaling{Implementation::x11DpiScaling()};
            if(!dpiScaling.isZero()) {
                Debug{verbose} << "Platform::SDLWindow: virtual DPI scaling" << dpiScaling.x();
                return dpiScaling;
            }
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
            if(!Implementation::isWindowsAppDpiAware()) {
                Warning{verbose} << "Platform::SDLWindow: your application is not set as DPI-aware, DPI scaling won't be used";
                return Vector2{1.0f};
            }
            Vector2 dpi;
            if(SDL_GetDisplayDPI(0, nullptr, &dpi.x(), &dpi.y()) == 0) {
                const Vector2 dpiScaling{dpi/96.0f};
                Debug{verbose} << "Platform::SDLWindow: virtual DPI scaling" << dpiScaling;
                return dpiScaling;
            }
#else
            Debug{verbose} << "Platform::SDLWindow: sorry, virtual DPI scaling not implemented on this platform yet, falling back to physical DPI scaling";
#endif
        }
#endif
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_ANDROID)
        CORRADE_INTERNAL_ASSERT(dpiScalingPolicy == Implementation::SimpleSdl2DpiScalingPolicy::Virtual || dpiScalingPolicy == Implementation::SimpleSdl2DpiScalingPolicy::Physical);
#else
        CORRADE_INTERNAL_ASSERT(dpiScalingPolicy == Implementation::SimpleSdl2DpiScalingPolicy::Physical);
#endif
#ifdef CORRADE_TARGET_EMSCRIPTEN
        const Vector2 dpiScaling{Implementation::emscriptenDpiScaling()};
        Debug{verbose} << "Platform::SDLWindow: physical DPI scaling" << dpiScaling.x();
        return dpiScaling;
#elif defined(CORRADE_TARGET_UNIX)
#if SDL_VERSION_ATLEAST(2, 0, 4)
        Vector2 dpi;
        if(SDL_GetDisplayDPI(0, nullptr, &dpi.x(), &dpi.y()) == 0) {
            const Vector2 dpiScaling{dpi/96.0f};
            Debug{verbose} << "Platform::SDLWindow: physical DPI scaling" << dpiScaling;
            return dpiScaling;
        }

        Warning{} << "Platform::SDLWindow: can't get physical display DPI, falling back to no scaling:" << SDL_GetError();
#else
        Debug{verbose} << "Platform::SDLWindow: sorry, physical DPI scaling only available on SDL 2.0.4+";
#endif
        return Vector2{1.0f};
#elif defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)
        HDC hDC = GetWindowDC(nullptr);
        Vector2i monitorSize{GetDeviceCaps(hDC, HORZSIZE), GetDeviceCaps(hDC, VERTSIZE)};
        SDL_DisplayMode mode;
        CORRADE_INTERNAL_ASSERT(SDL_GetDesktopDisplayMode(0, &mode) == 0);
        auto dpi = Vector2{Vector2i{mode.w, mode.h}*25.4f/Vector2{monitorSize}};
        const Vector2 dpiScaling{dpi/96.0f};
        Debug{verbose} << "Platform::SDLWindow: physical DPI scaling" << dpiScaling;
        return dpiScaling;
#else
        Debug{verbose} << "Platform::SDLWindow: sorry, physical DPI scaling not implemented on this platform yet";
        return Vector2{1.0f};
#endif
#endif
    }

    void SDLWindow::setWindowTitle(const std::string& title) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_SetWindowTitle(_window, title.data());
#else
        SDL_SetWindowTitle(nullptr, title.data());
#endif
    }

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && SDL_MAJOR_VERSION*1000 + SDL_MINOR_VERSION*100 + SDL_PATCHLEVEL >= 2005
    void SDLWindow::setWindowIcon(const ImageView2D& image) {
        Uint32 format;
        switch(image.format()) {
        case PixelFormat::RGB8Srgb:
        case PixelFormat::RGB8Unorm:
            format = SDL_PIXELFORMAT_RGB24;
            break;
        case PixelFormat::RGBA8Srgb:
        case PixelFormat::RGBA8Unorm:
            format = SDL_PIXELFORMAT_RGBA32;
            break;
        default:
            CORRADE_ASSERT_UNREACHABLE("Platform::SDLWindow::setWindowIcon(): unexpected format" << image.format(), );
        }
        Containers::StridedArrayView3D<const char> pixels = image.pixels().flipped<0>();
        SDL_Surface* icon = SDL_CreateRGBSurfaceWithFormatFrom(const_cast<void*>(pixels.data()) , image.size().x(), image.size().y(), 32, pixels.stride()[0], format);
        SDL_SetWindowIcon(_window, icon);
        SDL_FreeSurface(icon);
    }
#endif

    bool SDLWindow::tryCreate(const Configuration& configuration) {
#ifdef MAGNUM_TARGET_GL
        if(!(configuration.windowFlags() & Configuration::WindowFlag::Contextless))
            return tryCreate(configuration, GLConfiguration{});
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
        _dpiScaling = dpiScaling(configuration);
        const Vector2i scaledWindowSize = configuration.size()*_dpiScaling;
        if(!(_window = SDL_CreateWindow(
#ifndef CORRADE_TARGET_IOS
            configuration.title().data(),
#else
            nullptr,
#endif
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            scaledWindowSize.x(), scaledWindowSize.y(),
            SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL|Uint32(configuration.windowFlags() & ~Configuration::WindowFlag::Contextless))))
        {
            Error() << "Platform::SDLWindow::tryCreate(): cannot create window:" << SDL_GetError();
            return false;
        }
#else
        {
            Vector2d canvasSize;
            emscripten_get_element_css_size("#canvas", &canvasSize.x(), &canvasSize.y());
            _lastKnownCanvasSize = Vector2i{canvasSize};
        }
        Vector2i windowSize;
        if(!configuration.size().isZero()) {
            windowSize = configuration.size();
        } else {
            windowSize = _lastKnownCanvasSize;
            Debug{_verboseLog ? Debug::output() : nullptr} << "Platform::SDLWindow::tryCreate(): autodetected canvas size" << windowSize;
        }
        _dpiScaling = dpiScaling(configuration);
        const Vector2i scaledWindowSize = windowSize*_dpiScaling;

        Uint32 flags = SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF;
        if(configuration.windowFlags() & Configuration::WindowFlag::Resizable) {
            _flags |= Flag::Resizable;
            flags |= SDL_RESIZABLE;
        }

        if(!(_surface = SDL_SetVideoMode(scaledWindowSize.x(), scaledWindowSize.y(), 24, flags))) {
            Error() << "Platform::SDLWindow::tryCreate(): cannot create context:" << SDL_GetError();
            return false;
        }
#endif

        return true;
    }

#ifdef MAGNUM_TARGET_GL
    bool SDLWindow::tryCreate(const Configuration& configuration, const GLConfiguration& glConfiguration) {
        CORRADE_ASSERT(_context->version() == GL::Version::None, "Platform::SDLWindow::tryCreate(): context already created", false);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, glConfiguration.colorBufferSize().r());
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, glConfiguration.colorBufferSize().g());
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, glConfiguration.colorBufferSize().b());
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, glConfiguration.colorBufferSize().a());
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, glConfiguration.depthBufferSize());
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, glConfiguration.stencilBufferSize());
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, glConfiguration.sampleCount() > 1 ? 1 : 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, glConfiguration.sampleCount());

#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, glConfiguration.isSrgbCapable());
#endif
#ifndef CORRADE_TARGET_EMSCRIPTEN
        _dpiScaling = dpiScaling(configuration);
        const Vector2i scaledWindowSize = configuration.size()*_dpiScaling;
        GLConfiguration::Flags glFlags = glConfiguration.flags();
        if(_context->internalFlags() & GL::Context::InternalFlag::GpuValidation)
            glFlags |= GLConfiguration::Flag::Debug;
        if(glConfiguration.version() != GL::Version::None) {
            Int major, minor;
            std::tie(major, minor) = version(glConfiguration.version());
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);

#ifndef MAGNUM_TARGET_GLES
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, glConfiguration.version() >= GL::Version::GL310 ?
                SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, int(glFlags));
        } else {
#ifndef MAGNUM_TARGET_GLES
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#ifdef CORRADE_TARGET_APPLE
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, int(glFlags));
#else
#ifdef MAGNUM_TARGET_GLES3
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#elif defined(MAGNUM_TARGET_GLES2)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#else
#error unsupported OpenGL ES version
#endif
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
        }
        if(!(_window = SDL_CreateWindow(
#ifndef CORRADE_TARGET_IOS
            configuration.title().data(),
#else
            nullptr,
#endif
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            scaledWindowSize.x(), scaledWindowSize.y(),
            SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN|SDL_WINDOW_ALLOW_HIGHDPI|Uint32(configuration.windowFlags()))))
        {
            Error() << "Platform::SDLWindow::tryCreate(): cannot create window:" << SDL_GetError();
            return false;
        }
        _glContext = SDL_GL_CreateContext(_window);

#ifndef MAGNUM_TARGET_GLES
#ifndef CORRADE_TARGET_APPLE
        constexpr static const char nvidiaVendorString[] = "NVIDIA Corporation";
#ifdef CORRADE_TARGET_WINDOWS
        constexpr static const char intelVendorString[] = "Intel";
#endif
        constexpr static const char amdVendorString[] = "ATI Technologies Inc.";
        const char* vendorString;
#endif
        if(glConfiguration.version() == GL::Version::None && (!_glContext
#ifndef CORRADE_TARGET_APPLE
            || (vendorString = reinterpret_cast<const char*>(glGetString(GL_VENDOR)),
                vendorString && (std::strncmp(vendorString, nvidiaVendorString, sizeof(nvidiaVendorString)) == 0 ||
#ifdef CORRADE_TARGET_WINDOWS
                    std::strncmp(vendorString, intelVendorString, sizeof(intelVendorString)) == 0 ||
#endif
                    std::strncmp(vendorString, amdVendorString, sizeof(amdVendorString)) == 0)
                && !_context->isDriverWorkaroundDisabled("no-forward-compatible-core-context"))
#endif
            )) {
            if(!_glContext) Warning()
                << "Platform::SDLWindow::tryCreate(): cannot create core context:"
                << SDL_GetError() << "(falling back to compatibility context)";
            else SDL_GL_DeleteContext(_glContext);

            SDL_DestroyWindow(_window);

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, int(glFlags) & int(~GLConfiguration::Flag::ForwardCompatible));

            if(!(_window = SDL_CreateWindow(configuration.title().data(),
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                scaledWindowSize.x(), scaledWindowSize.y(),
                SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN|SDL_WINDOW_ALLOW_HIGHDPI|Uint32(configuration.windowFlags()&~Configuration::WindowFlag::Contextless))))
            {
                Error() << "Platform::SDLWindow::tryCreate(): cannot create window:" << SDL_GetError();
                return false;
            }
            _glContext = SDL_GL_CreateContext(_window);
        }
#endif
        if(!_glContext) {
            Error() << "Platform::SDLWindow::tryCreate(): cannot create context:" << SDL_GetError();
            SDL_DestroyWindow(_window);
            _window = nullptr;
            return false;
        }

#ifdef CORRADE_TARGET_IOS
        {
            const Vector2i viewport = framebufferSize();
            glViewport(0, 0, viewport.x(), viewport.y());
        }
#endif
#else
        {
            Vector2d canvasSize;
            emscripten_get_element_css_size("#canvas", &canvasSize.x(), &canvasSize.y());
            _lastKnownCanvasSize = Vector2i{canvasSize};
        }
        Vector2i windowSize;
        if(!configuration.size().isZero()) {
            windowSize = configuration.size();
        } else {
            windowSize = _lastKnownCanvasSize;
            Debug{_verboseLog ? Debug::output() : nullptr} << "Platform::SDLWindow::tryCreate(): autodetected canvas size" << windowSize;
        }
        _dpiScaling = dpiScaling(configuration);
        const Vector2i scaledWindowSize = windowSize*_dpiScaling;

        Uint32 flags = SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF;
        if(configuration.windowFlags() & Configuration::WindowFlag::Resizable) {
            _flags |= Flag::Resizable;
            flags |= SDL_RESIZABLE;
        }

        if(!(_surface = SDL_SetVideoMode(scaledWindowSize.x(), scaledWindowSize.y(), 24, flags))) {
            Error() << "Platform::SDLWindow::tryCreate(): cannot create context:" << SDL_GetError();
            return false;
        }
#endif
        if(!_context->tryCreate()) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
            SDL_GL_DeleteContext(_glContext);
            SDL_DestroyWindow(_window);
            _window = nullptr;
#else
            SDL_FreeSurface(_surface);
#endif
            return false;
        }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        if(!(configuration.windowFlags() & Configuration::WindowFlag::Hidden))
            SDL_ShowWindow(_window);
#endif
        return true;
    }
#endif

    Vector2i SDLWindow::windowSize() const {
        Vector2i size;
#ifndef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_ASSERT(_window, "Platform::SDLWindow::windowSize(): no window opened", {});
        SDL_GetWindowSize(_window, &size.x(), &size.y());
#else
        CORRADE_ASSERT(_surface, "Platform::SDLWindow::windowSize(): no window opened", {});
        emscripten_get_canvas_element_size("#canvas", &size.x(), &size.y());
#endif
        return size;
    }

#ifndef CORRADE_TARGET_EMSCRIPTEN
    void SDLWindow::setWindowSize(const Vector2i& size) {
        CORRADE_ASSERT(_window, "Platform::SDLWindow::setWindowSize(): no window opened", );

        const Vector2i newSize = _dpiScaling*size;
        SDL_SetWindowSize(_window, newSize.x(), newSize.y());
    }

    void SDLWindow::setMinWindowSize(const Vector2i& size) {
        CORRADE_ASSERT(_window, "Platform::SDLWindow::setMinWindowSize(): no window opened", );

        const Vector2i newSize = _dpiScaling*size;
        SDL_SetWindowMinimumSize(_window, newSize.x(), newSize.y());
    }

    void SDLWindow::setMaxWindowSize(const Vector2i& size) {
        CORRADE_ASSERT(_window, "Platform::SDLWindow::setMaxWindowSize(): no window opened", );

        const Vector2i newSize = _dpiScaling*size;
        SDL_SetWindowMaximumSize(_window, newSize.x(), newSize.y());
    }
#endif

#ifdef MAGNUM_TARGET_GL
    Vector2i SDLWindow::framebufferSize() const {
        Vector2i size;
#ifndef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_ASSERT(_window, "Platform::SDLWindow::framebufferSize(): no window opened", {});
        SDL_GL_GetDrawableSize(_window, &size.x(), &size.y());
#else
        CORRADE_ASSERT(_surface, "Platform::SDLWindow::framebufferSize(): no window opened", {});
        emscripten_get_canvas_element_size("#canvas", &size.x(), &size.y());
#endif
        return size;
    }
#endif

#ifdef CORRADE_TARGET_EMSCRIPTEN
    void SDLWindow::setContainerCssClass(const std::string& cssClass) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
        EM_ASM_({document.getElementById('container').className = AsciiToString($0);}, cssClass.data());
#pragma GCC diagnostic pop
    }
#endif

    void SDLWindow::swapBuffers() {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_GL_SwapWindow(_window);
#else
        SDL_Flip(_surface);
#endif
    }

    Int SDLWindow::swapInterval() const {
        return SDL_GL_GetSwapInterval();
    }

    bool SDLWindow::setSwapInterval(const Int interval) {
        if(SDL_GL_SetSwapInterval(interval) == -1) {
            Error() << "Platform::SDLWindow::setSwapInterval(): cannot set swap interval:" << SDL_GetError();
            _flags &= ~Flag::VSyncEnabled;
            return false;
        }

        if(SDL_GL_GetSwapInterval() != interval) {
            Error() << "Platform::SDLWindow::setSwapInterval(): swap interval setting ignored by the driver";
            _flags &= ~Flag::VSyncEnabled;
            return false;
        }

        if(interval) _flags |= Flag::VSyncEnabled;
        else _flags &= ~Flag::VSyncEnabled;
        return true;
    }

    void SDLWindow::redraw() { _flags |= Flag::Redraw; }

    SDLWindow::~SDLWindow() {

#ifdef MAGNUM_TARGET_GL
        _context.reset();

#ifndef CORRADE_TARGET_EMSCRIPTEN
        if(_glContext) SDL_GL_DeleteContext(_glContext);
#else
        if(_surface) SDL_FreeSurface(_surface);
#endif
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
        for(auto& cursor: _cursors)
            SDL_FreeCursor(cursor);
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
        if(_window) SDL_DestroyWindow(_window);
#endif
        SDL_Quit();
    }

    int SDLWindow::exec() {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        while(mainLoopIteration()) {}
#else
        emscripten_set_main_loop_arg([](void* arg) {
            static_cast<SDLWindow*>(arg)->mainLoopIteration();
            }, this, 0, true);
#endif
        return _exitCode;
    }

    void SDLWindow::exit(const int exitCode) {
        _flags |= Flag::Exit;
#ifdef CORRADE_TARGET_EMSCRIPTEN
        emscripten_cancel_main_loop();
#endif
        _exitCode = exitCode;
    }
    bool SDLWindow::mainLoopIteration()
    {
        if(_flags & Flag::Exit) return false;

#ifndef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_ASSERT(_window, "Platform::SDLWindow::mainLoopIteration(): no window opened", {});
#else
        CORRADE_ASSERT(_surface, "Platform::SDLWindow::mainLoopIteration(): no window opened", {});
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
        const UnsignedInt timeBefore = _minimalLoopPeriod ? SDL_GetTicks() : 0;
#endif

#ifdef CORRADE_TARGET_EMSCRIPTEN
        if(_flags & Flag::Resizable) {
            Vector2d canvasSize;
            emscripten_get_element_css_size("#canvas", &canvasSize.x(), &canvasSize.y());

            const Vector2i canvasSizei{canvasSize};
            if(canvasSizei != _lastKnownCanvasSize) {
                _lastKnownCanvasSize = canvasSizei;
                const Vector2i size = _dpiScaling*canvasSizei;
                emscripten_set_canvas_element_size("#canvas", size.x(), size.y());
                ViewportEvent e{
#ifdef MAGNUM_TARGET_GL
                    size,
#endif
                    size, _dpiScaling};
                viewportEvent(e);
                _flags |= Flag::Redraw;
            }
        }
#endif

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
#ifdef CORRADE_TARGET_EMSCRIPTEN
                    CORRADE_INTERNAL_ASSERT_UNREACHABLE();
#else
                    ViewportEvent e{event, windowSize(),
#ifdef MAGNUM_TARGET_GL
                        framebufferSize(),
#endif
                        _dpiScaling};
                    viewportEvent(e);
                    _flags |= Flag::Redraw;
#endif
                } break;
                case SDL_WINDOWEVENT_EXPOSED:
                    _flags |= Flag::Redraw;
                    if(!(_flags & Flag::NoAnyEvent)) anyEvent(event);
                    break;
                default:
                    if(!(_flags & Flag::NoAnyEvent)) anyEvent(event);
                } break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                KeyEvent e{event, static_cast<KeyEvent::Key>(event.key.keysym.sym), fixedModifiers(event.key.keysym.mod), event.key.repeat != 0};
                event.type == SDL_KEYDOWN ? keyPressEvent(e) : keyReleaseEvent(e);
            } break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                MouseEvent e{event, static_cast<MouseEvent::Button>(event.button.button), {event.button.x, event.button.y}
#ifndef CORRADE_TARGET_EMSCRIPTEN
                    , event.button.clicks
#endif
                };
                event.type == SDL_MOUSEBUTTONDOWN ? mousePressEvent(e) : mouseReleaseEvent(e);
            } break;

            case SDL_MOUSEWHEEL: {
                MouseScrollEvent e{event, {Float(event.wheel.x), Float(event.wheel.y)}};
                mouseScrollEvent(e);
            } break;

            case SDL_MOUSEMOTION: {
                MouseMoveEvent e{event, {event.motion.x, event.motion.y}, {event.motion.xrel, event.motion.yrel}, static_cast<MouseMoveEvent::Button>(event.motion.state)};
                mouseMoveEvent(e);
                break;
            }

            case SDL_MULTIGESTURE: {
                MultiGestureEvent e{event, {event.mgesture.x, event.mgesture.y}, event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.numFingers};
                multiGestureEvent(e);
                break;
            }

            case SDL_TEXTINPUT: {
                TextInputEvent e{event, {event.text.text, std::strlen(event.text.text)}};
                textInputEvent(e);
            } break;

            case SDL_TEXTEDITING: {
                TextEditingEvent e{event, {event.edit.text, std::strlen(event.text.text)}, event.edit.start, event.edit.length};
                textEditingEvent(e);
            } break;

            case SDL_QUIT: {
                ExitEvent e{event};
                exitEvent(e);
                if(e.isAccepted()) {
                    _flags |= Flag::Exit;
#ifdef CORRADE_TARGET_EMSCRIPTEN
                    emscripten_cancel_main_loop();
#endif
                    return !(_flags & Flag::Exit);
                }
            } break;
            default: if(!(_flags & Flag::NoAnyEvent)) anyEvent(event);
            }

            
            input(event);
        }
        if(!(_flags & Flag::NoTickEvent)) tickEvent();
        if(_flags & Flag::Redraw) {
            _flags &= ~Flag::Redraw;
            drawEvent();

#ifndef CORRADE_TARGET_EMSCRIPTEN
            if(!(_flags & Flag::VSyncEnabled) && _minimalLoopPeriod) {
                const UnsignedInt loopTime = SDL_GetTicks() - timeBefore;
                if(loopTime < _minimalLoopPeriod)
                    SDL_Delay(_minimalLoopPeriod - loopTime);
            }
#endif

            return !(_flags & Flag::Exit);
        }

#ifndef CORRADE_TARGET_EMSCRIPTEN
        if(_minimalLoopPeriod) {
            const UnsignedInt loopTime = SDL_GetTicks() - timeBefore;
            if(loopTime < _minimalLoopPeriod)
                SDL_Delay(_minimalLoopPeriod - loopTime);
        }
        if(_flags & Flag::NoTickEvent) SDL_WaitEvent(nullptr);
#endif
        return !(_flags & Flag::Exit);
    }

    namespace {

#ifndef CORRADE_TARGET_EMSCRIPTEN
        constexpr SDL_SystemCursor CursorMap[] {
            SDL_SYSTEM_CURSOR_ARROW,
            SDL_SYSTEM_CURSOR_IBEAM,
            SDL_SYSTEM_CURSOR_WAIT,
            SDL_SYSTEM_CURSOR_CROSSHAIR,
            SDL_SYSTEM_CURSOR_WAITARROW,
            SDL_SYSTEM_CURSOR_SIZENWSE,
            SDL_SYSTEM_CURSOR_SIZENESW,
            SDL_SYSTEM_CURSOR_SIZEWE,
            SDL_SYSTEM_CURSOR_SIZENS,
            SDL_SYSTEM_CURSOR_SIZEALL,
            SDL_SYSTEM_CURSOR_NO,
            SDL_SYSTEM_CURSOR_HAND
        };
#else
        constexpr const char* CursorMap[] {
            "default",
            "text",
            "wait",
            "crosshair",
            "progress",
            "nwse-resize",
            "nesw-resize",
            "ew-resize",
            "ns-resize",
            "move",
            "not-allowed",
            "pointer",
            "none"
        };
#endif

    }

    void SDLWindow::setCursor(Cursor cursor) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        CORRADE_INTERNAL_ASSERT(UnsignedInt(cursor) < Containers::arraySize(_cursors));

        if(cursor == Cursor::Hidden) {
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetWindowGrab(_window, SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
            return;
        } else if(cursor == Cursor::HiddenLocked) {
            SDL_SetWindowGrab(_window, SDL_TRUE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
            return;
        } else {
            SDL_ShowCursor(SDL_ENABLE);
            SDL_SetWindowGrab(_window, SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }

        if(!_cursors[UnsignedInt(cursor)])
            _cursors[UnsignedInt(cursor)] = SDL_CreateSystemCursor(CursorMap[UnsignedInt(cursor)]);

        SDL_SetCursor(_cursors[UnsignedInt(cursor)]);
#else
        _cursor = cursor;
        CORRADE_INTERNAL_ASSERT(UnsignedInt(cursor) < Containers::arraySize(CursorMap));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
        EM_ASM_({document.getElementById('canvas').style.cursor = AsciiToString($0);}, CursorMap[UnsignedInt(cursor)]);
#pragma GCC diagnostic pop
#endif
    }

    SDLWindow::Cursor SDLWindow::cursor() {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        if(SDL_GetRelativeMouseMode())
            return Cursor::HiddenLocked;
        else if(!SDL_ShowCursor(SDL_QUERY))
            return Cursor::Hidden;

        SDL_Cursor* cursor = SDL_GetCursor();

        if(cursor) for(UnsignedInt i = 0; i < sizeof(_cursors); i++)
            if(_cursors[i] == cursor) return Cursor(i);

        return Cursor::Arrow;
#else
        return _cursor;
#endif
    }

#ifdef MAGNUM_BUILD_DEPRECATED
    void SDLWindow::setMouseLocked(bool enabled) {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        SDL_SetWindowGrab(_window, enabled ? SDL_TRUE : SDL_FALSE);
        SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
#else
        CORRADE_ASSERT_UNREACHABLE("SDLWindow::setMouseLocked(): not implemented", );
        static_cast<void>(enabled);
#endif
    }
#endif

    bool SDLWindow::isTextInputActive() {
#ifndef CORRADE_TARGET_EMSCRIPTEN
        return SDL_IsTextInputActive();
#else
        return !!(_flags & Flag::TextInputActive);
#endif
    }

    void SDLWindow::startTextInput() {
        SDL_StartTextInput();
#ifdef CORRADE_TARGET_EMSCRIPTEN
        _flags |= Flag::TextInputActive;
#endif
    }

    void SDLWindow::stopTextInput() {
        SDL_StopTextInput();
#ifdef CORRADE_TARGET_EMSCRIPTEN
        _flags &= ~Flag::TextInputActive;
#endif
    }

    void SDLWindow::setTextInputRect(const Range2Di& rect) {
        SDL_Rect r{rect.min().x(), rect.min().y(), rect.sizeX(), rect.sizeY()};
        SDL_SetTextInputRect(&r);
    }

    void SDLWindow::exitEvent(ExitEvent& event) {
        event.setAccepted();
    }

    void SDLWindow::tickEvent() {
        _flags |= Flag::NoTickEvent;
    }

    void SDLWindow::anyEvent(SDL_Event&) {
        _flags |= Flag::NoAnyEvent;
    }

    void SDLWindow::viewportEvent(ViewportEvent& event) {
#ifdef MAGNUM_BUILD_DEPRECATED
        CORRADE_IGNORE_DEPRECATED_PUSH
            viewportEvent(
#ifdef MAGNUM_TARGET_GL
                event.framebufferSize()
#else
                event.windowSize()
#endif
            );
        CORRADE_IGNORE_DEPRECATED_POP
#else
        static_cast<void>(event);
#endif
    }

#ifdef MAGNUM_BUILD_DEPRECATED
    void SDLWindow::viewportEvent(const Vector2i&) {}
#endif

    void SDLWindow::keyPressEvent(KeyEvent&) {}
    void SDLWindow::keyReleaseEvent(KeyEvent&) {}
    void SDLWindow::mousePressEvent(MouseEvent&) {}
    void SDLWindow::mouseReleaseEvent(MouseEvent&) {}
    void SDLWindow::mouseMoveEvent(MouseMoveEvent&) {}
    void SDLWindow::mouseScrollEvent(MouseScrollEvent&) {}
    void SDLWindow::multiGestureEvent(MultiGestureEvent&) {}
    void SDLWindow::textInputEvent(TextInputEvent&) {}
    void SDLWindow::textEditingEvent(TextEditingEvent&) {}

#ifdef MAGNUM_TARGET_GL
    SDLWindow::GLConfiguration::GLConfiguration():
        _colorBufferSize{8, 8, 8, 8}, _depthBufferSize{24}, _stencilBufferSize{0},
        _sampleCount(0)
#ifndef CORRADE_TARGET_EMSCRIPTEN
        , _version(GL::Version::None),
#ifndef MAGNUM_TARGET_GLES
        _flags{Flag::ForwardCompatible},
#else
        _flags{},
#endif
        _srgbCapable{false}
#endif
    {}

    SDLWindow::GLConfiguration::~GLConfiguration() = default;
#endif

    SDLWindow::Configuration::Configuration():
#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_IOS)
        _title("Magnum SDL2 Application"),
#endif
#if !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        _size{800, 600},
#else
        _size{},
#endif
        _dpiScalingPolicy{DpiScalingPolicy::Default} {}

    SDLWindow::Configuration::~Configuration() = default;

    std::string SDLWindow::KeyEvent::keyName(const Key key) {
        return SDL_GetKeyName(SDL_Keycode(key));
    }

    std::string SDLWindow::KeyEvent::keyName() const {
        return keyName(_key);
    }

    SDLWindow::InputEvent::Modifiers SDLWindow::MouseEvent::modifiers() {
        if(_modifiers) return *_modifiers;
        return *(_modifiers = fixedModifiers(Uint16(SDL_GetModState())));
    }

    SDLWindow::InputEvent::Modifiers SDLWindow::MouseMoveEvent::modifiers() {
        if(_modifiers) return *_modifiers;
        return *(_modifiers = fixedModifiers(Uint16(SDL_GetModState())));
    }

    Vector2i SDLWindow::MouseScrollEvent::position() {
        if(_position) return *_position;
        _position = Vector2i{};
        SDL_GetMouseState(&_position->x(), &_position->y());
        return *_position;
    }

    SDLWindow::InputEvent::Modifiers SDLWindow::MouseScrollEvent::modifiers() {
        if(_modifiers) return *_modifiers;
        return *(_modifiers = fixedModifiers(Uint16(SDL_GetModState())));
    }

    template class BasicScreen<SDLWindow>;
    template class BasicScreenedApplication<SDLWindow>;

}}
