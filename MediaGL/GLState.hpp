#pragma once

struct Shader;
struct Texture;
struct VertexArray;
struct VertexBuffer;
struct ElementBuffer;
struct SSBO;

struct GLState
{
    Shader*        boundShader        = nullptr;
    Texture*       boundTexture       = nullptr;
    VertexArray*   boundVertexArray   = nullptr;
    VertexBuffer*  boundVertexBuffer  = nullptr;
    ElementBuffer* boundElementBuffer = nullptr;
    SSBO*          boundSSBO          = nullptr;

    void reset() { *this = GLState(); }
};

GLState glState;