#pragma once

#pragma comment(lib, "OpenGL32.lib")

#include <TLib/Media/Platform/SDL2.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Logging.hpp>

#define GL_GLEXT_PROTOTYPES
#include <gl/gl3w.h>
#include <gl/GL.h>
#include <cstdint>

static auto glSink = tlog::createConsoleLogger("GL");

enum class GLType : int
{
    // https://www.khronos.org/opengl/wiki/OpenGL_Type
    Unknown [[maybe_unused]] = -1                   ,
    Bool    [[maybe_unused]] = GL_BOOL              ,
    Byte    [[maybe_unused]] = GL_BYTE              ,
    UByte   [[maybe_unused]] = GL_UNSIGNED_BYTE     ,
    Short   [[maybe_unused]] = GL_SHORT             ,
    UShort  [[maybe_unused]] = GL_UNSIGNED_SHORT    ,
    Int     [[maybe_unused]] = GL_INT               ,
    UInt    [[maybe_unused]] = GL_UNSIGNED_INT      ,
    Float   [[maybe_unused]] = GL_FLOAT             ,
    HFloat  [[maybe_unused]] = GL_HALF_FLOAT        ,
    Double  [[maybe_unused]] = GL_DOUBLE            ,
    Fixed   [[maybe_unused]] = GL_FIXED             ,
};

static std::unordered_map<GLType, uint32_t> glTypeSizeMap =
{
    { GLType::Bool   , static_cast<uint32_t>( sizeof(int8_t)      ) },
    { GLType::Byte   , static_cast<uint32_t>( sizeof(int8_t)      ) },
    { GLType::UByte  , static_cast<uint32_t>( sizeof(uint8_t)     ) },
    { GLType::Short  , static_cast<uint32_t>( sizeof(int16_t)     ) },
    { GLType::UShort , static_cast<uint32_t>( sizeof(uint16_t)    ) },
    { GLType::Int    , static_cast<uint32_t>( sizeof(int32_t)     ) },
    { GLType::UInt   , static_cast<uint32_t>( sizeof(uint32_t)    ) },
    { GLType::Float  , static_cast<uint32_t>( sizeof(float_t)     ) },
    { GLType::HFloat , static_cast<uint32_t>( sizeof(float_t) / 2 ) },
    { GLType::Double , static_cast<uint32_t>( sizeof(double_t)    ) },
    { GLType::Fixed  , static_cast<uint32_t>( sizeof(float_t)     ) }
};

enum class GLDrawMode : GLenum
{
    Points                 [[maybe_unused]] = GL_POINTS                       ,
    LineStrip              [[maybe_unused]] = GL_LINE_STRIP                   ,
    LineLoop               [[maybe_unused]] = GL_LINE_LOOP                    ,
    Lines                  [[maybe_unused]] = GL_LINES                        ,
    LineStripAdjacency     [[maybe_unused]] = GL_LINE_STRIP_ADJACENCY         ,
    LinesAdjacency         [[maybe_unused]] = GL_LINES_ADJACENCY              ,
    TriangleStrip          [[maybe_unused]] = GL_TRIANGLE_STRIP               ,
    TriangleFan            [[maybe_unused]] = GL_TRIANGLE_FAN                 ,
    Triangles              [[maybe_unused]] = GL_TRIANGLES                    ,
    TriangleStripAdjacency [[maybe_unused]] = GL_TRIANGLE_STRIP_ADJACENCY     ,
    TrianglesAdjacency     [[maybe_unused]] = GL_TRIANGLES_ADJACENCY          ,
    Patches                [[maybe_unused]] = GL_PATCHES
};

enum class GLBlendMode : GLenum
{
    Zero                  [[maybe_unused]] = GL_ZERO                         ,
    One                   [[maybe_unused]] = GL_ONE                          ,
    SrcColor              [[maybe_unused]] = GL_SRC_COLOR                    ,
    OneMinusSrcColor      [[maybe_unused]] = GL_ONE_MINUS_SRC_COLOR          ,
    DstColor              [[maybe_unused]] = GL_DST_COLOR                    ,
    OneMinusDstColor      [[maybe_unused]] = GL_ONE_MINUS_DST_COLOR          ,
    SrcAlpha              [[maybe_unused]] = GL_SRC_ALPHA                    ,
    OneMinusSrcAlpha      [[maybe_unused]] = GL_ONE_MINUS_SRC_ALPHA          ,
    DstAlpha              [[maybe_unused]] = GL_DST_ALPHA                    ,
    OneMinusDstAlpha      [[maybe_unused]] = GL_ONE_MINUS_DST_ALPHA          ,
    ConstantColor         [[maybe_unused]] = GL_CONSTANT_COLOR               ,
    OneMinusConstantColor [[maybe_unused]] = GL_ONE_MINUS_CONSTANT_COLOR     ,
    ConstantAlpha         [[maybe_unused]] = GL_CONSTANT_ALPHA               ,
    OneMinusConstantAlpha [[maybe_unused]] = GL_ONE_MINUS_CONSTANT_ALPHA
};

enum class VSyncMode
{
    Disabled [[maybe_unused]] =  0,
    Enabled  [[maybe_unused]] =  1,
    Adaptive [[maybe_unused]] = -1
};

enum class FaceCullMode
{
    None,
    Front,
    Back,
    Both
};

static const char* glErrorTypeToStr(GLenum type)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR               : return "GL_DEBUG_TYPE_ERROR - An error, typically from the API";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR - Some behavior marked deprecated has been used";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  : return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR - Something has invoked undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY 	       : return "GL_DEBUG_TYPE_PORTABILITY - Some functionality the user relies upon is not portable";
    case GL_DEBUG_TYPE_PERFORMANCE 	       : return "GL_DEBUG_TYPE_PERFORMANCE - Code has triggered possible performance issues";
    case GL_DEBUG_TYPE_MARKER 	           : return "GL_DEBUG_TYPE_MARKER - Command stream annotation";
    case GL_DEBUG_TYPE_PUSH_GROUP 	       : return "GL_DEBUG_TYPE_PUSH_GROUP - Group pushing";
    case GL_DEBUG_TYPE_POP_GROUP 	       : return "GL_DEBUG_TYPE_POP_GROUP - Group popping";
    case GL_DEBUG_TYPE_OTHER               : return "GL_DEBUG_TYPE_OTHER";
    default: return "Unknown"; break;
    };
}

static const char* glErrorSourceToStr(GLenum source)
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API             : return "GL_DEBUG_SOURCE_API - Calls to the OpenGL API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM   : return "GL_DEBUG_SOURCE_WINDOW_SYSTEM - Calls to a window-system API";
    case GL_DEBUG_SOURCE_SHADER_COMPILER : return "GL_DEBUG_SOURCE_SHADER_COMPILER - A compiler for a shading language";
    case GL_DEBUG_SOURCE_THIRD_PARTY     : return "GL_DEBUG_SOURCE_THIRD_PARTY - An application associated with OpenGL";
    case GL_DEBUG_SOURCE_APPLICATION     : return "GL_DEBUG_SOURCE_APPLICATION - Generated by the user of this application";
    case GL_DEBUG_SOURCE_OTHER           : return "GL_DEBUG_SOURCE_OTHER";
    default: return "Unknown"; break;
    }
}

// https://www.khronos.org/opengl/wiki/Debug_Output#Examples
static void defaultGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam)
{
    String str = fmt::format(R"(
Source:  {}
Type:    {},
Message: {})", glErrorSourceToStr(source), glErrorTypeToStr(type), message);

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            /*glSink->info(str);*/ break;
        case GL_DEBUG_SEVERITY_LOW:
            glSink->warn(str); break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            glSink->error(str); break;
        case GL_DEBUG_SEVERITY_HIGH:
            glSink->critical(str); break;
        default: break;
    }
}

template <typename... FmtArgs>
static String formatShader(String str, FmtArgs... args)
{
    strhelp::replace(str, "{", "{{");
    strhelp::replace(str, "}", "}}");

    while (true)
    {
        if (strhelp::replaceFirst(str, "`", "{"))
        { strhelp::replaceFirst(str, "`", "}"); }
        else { break; }
    }

    return fmt::format(fmt::runtime(str), args...);
}

#pragma region GLCHECK

static void checkOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {
        glSink->critical("{}, at {}:{} - for {}\n", err, fname, line, stmt);
        ASSERT(false); // Only crash fast in debug pls
    }
}

static void checkOpenGLErrorNoAbort(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {
        glSink->critical("{}, at {}:{} - for {}\n", err, fname, line, stmt);
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            checkOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#ifdef _DEBUG
#define GL_CHECK_NOABORT(stmt) do { \
            stmt; \
            checkOpenGLErrorNoAbort(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK_NOABORT(stmt) stmt
#endif

#pragma endregion