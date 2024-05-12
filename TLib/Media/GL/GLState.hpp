#pragma once
#include <TLib/Containers/Vector.hpp>

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
    Vector<Texture*>      boundTextures      { 64, nullptr };
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