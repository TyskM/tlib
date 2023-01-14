#pragma once

#include <vector>

struct Shader;
struct Texture;
struct VertexArray;
struct VertexBuffer;
struct ElementBuffer;
struct SSBO;

struct GLState
{
    std::vector<Texture*> boundTextures      = {0, 0, 0, 0, 0, 0, 0, 0};
    Shader*               boundShader        = nullptr;
    VertexArray*          boundVertexArray   = nullptr;
    VertexBuffer*         boundVertexBuffer  = nullptr;
    ElementBuffer*        boundElementBuffer = nullptr;
    SSBO*                 boundSSBO          = nullptr;

    void reset() { *this = GLState(); }
};

GLState glState;