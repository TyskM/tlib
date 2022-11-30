#pragma once

struct Shader;
struct Texture;
struct VertexArray;

struct GLState
{
    Shader*      boundShader      = nullptr;
    Texture*     boundTexture     = nullptr;
    VertexArray* boundVertexArray = nullptr;

    void reset() { *this = GLState(); }
};

GLState glState;