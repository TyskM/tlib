#pragma once

#pragma comment(lib, "OpenGL32.lib")

#define GL_GLEXT_PROTOTYPES
#include <gl/gl3w.h>
#include <gl/GL.h>
#include "../Macros.hpp"
#include "../Logging.hpp"

void checkOpenGLError(const char* stmt, const char* fname, int line);
#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            checkOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

auto glSink = tlog::createConsoleLogger("OpenGL");

enum class GLType : int
{
    // https://www.khronos.org/opengl/wiki/OpenGL_Type
    Byte   = GL_BYTE,
    UByte  = GL_UNSIGNED_BYTE,
    Short  = GL_SHORT,
    UShort = GL_UNSIGNED_SHORT,
    Int    = GL_INT,
    UInt   = GL_UNSIGNED_INT,
    Float  = GL_FLOAT,
    HFloat = GL_HALF_FLOAT,
    Double = GL_DOUBLE,
    Fixed  = GL_FIXED,
};

std::unordered_map<GLType, size_t> glTypeSizeMap =
{
    { GLType::Byte   , sizeof(int8_t)      },
    { GLType::UByte  , sizeof(uint8_t)     },
    { GLType::Short  , sizeof(int16_t)     },
    { GLType::UShort , sizeof(uint16_t)    },
    { GLType::Int    , sizeof(int32_t)     },
    { GLType::UInt   , sizeof(uint32_t)    },
    { GLType::Float  , sizeof(float_t)     },
    { GLType::HFloat , sizeof(float_t) / 2 },
    { GLType::Double , sizeof(double_t)    },
    { GLType::Fixed  , sizeof(float_t)     }
};

void setVertexAttribPtr(int location, int count, GLType type,
                        bool normalized = false, size_t size = 0, void* offset = (void*)0)
{
    if (size == 0) { size = count * glTypeSizeMap[type]; }
    GL_CHECK(glVertexAttribPointer(location, count, static_cast<int>(type), normalized, static_cast<GLsizei>(size), offset));
    GL_CHECK(glEnableVertexAttribArray(location));
}

// https://www.khronos.org/opengl/wiki/Debug_Output#Examples
void defaultGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, const void* userParam)
{
    String str = fmt::format(R"(
OpenGL:
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

void checkOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {
        glSink->critical("{}, at {}:{} - for {}\n", err, fname, line, stmt);
        ASSERT(false); // Only crash fast in debug pls
    }
}
