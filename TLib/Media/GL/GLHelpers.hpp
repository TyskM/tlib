#pragma once

#pragma comment(lib, "OpenGL32.lib")

#include <TLib/Media/Platform/SDL2.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Logging.hpp>

#define GL_GLEXT_PROTOTYPES
#include <gl/gl3w.h>
#include <gl/GL.h>
#include <cstdint>

auto glSink = tlog::createConsoleLogger("GL");

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

std::unordered_map<GLType, uint32_t> glTypeSizeMap =
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

// https://www.khronos.org/opengl/wiki/Debug_Output#Examples
void defaultGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, const void* userParam)
{
    String str = fmt::format(R"(
GL:
Type:    {},
Message: {})", type, message);

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
String formatShader(String str, FmtArgs... args)
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

void checkOpenGLError(const char* stmt, const char* fname, int line);
#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            checkOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

void checkOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {
        glSink->critical("{}, at {}:{} - for {}\n", err, fname, line, stmt);
        ASSERT(false); // Only crash fast in debug pls
    }
}

#pragma endregion