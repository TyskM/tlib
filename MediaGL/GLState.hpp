#pragma once

struct Shader;
struct Texture;
struct VertexArray;
struct VertexBuffer;

struct GLState
{
    Shader*       boundShader       = nullptr;
    Texture*      boundTexture      = nullptr;
    VertexArray*  boundVertexArray  = nullptr;
    VertexBuffer* boundVertexBuffer = nullptr;

    void reset() { *this = GLState(); }
};

GLState glState;