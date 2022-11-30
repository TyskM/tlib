#pragma once

#include <gl/gl3w.h>
#include <gl/GL.h>
#include <iostream>

// https://www.khronos.org/opengl/wiki/Debug_Output#Examples
void defaultGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) { return; }
    fprintf( stderr,
        R"""(
OpenGL: %s
    Type: 0x%x,
    Severity: 0x%x,
    Message: %s
)""",
        ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
        type, severity, message );
}

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        //abort();
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif
