#pragma once

struct Shader;
struct Texture;
struct VertexArray;
struct VertexBuffer;
struct ElementBuffer;

struct GLState
{
    Shader*        boundShader        = nullptr;
    Texture*       boundTexture       = nullptr;
    VertexArray*   boundVertexArray   = nullptr;
    VertexBuffer*  boundVertexBuffer  = nullptr;
    ElementBuffer* boundElementBuffer = nullptr;

    void reset() { *this = GLState(); }
};

GLState glState;