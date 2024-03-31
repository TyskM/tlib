#pragma once

#include <vector>

struct Shader;
struct Texture;
struct VertexArray;
struct VertexBuffer;
struct ElementBuffer;
struct ShaderStorageBuffer;
struct UniformBuffer;
struct FrameBuffer;

struct GLState
{
    std::vector<Texture*> boundTextures      = {0, 0, 0, 0, 0, 0, 0, 0};
    Shader*               boundShader        = nullptr;
    VertexArray*          boundVertexArray   = nullptr;
    VertexBuffer*         boundVertexBuffer  = nullptr;
    ElementBuffer*        boundElementBuffer = nullptr;
    ShaderStorageBuffer*  boundSSBO          = nullptr;
    UniformBuffer*        boundUniformBuffer = nullptr;
    FrameBuffer*          boundFrameBuffer   = nullptr;

    void reset() { *this = GLState(); }
};

static GLState glState;