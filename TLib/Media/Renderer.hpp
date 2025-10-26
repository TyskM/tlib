#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/Shader.hpp>
#include <TLib/Media/GL/FrameBuffer.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Resource/GPUVertexData.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Containers/Span.hpp>
#include <TLib/Containers/Pair.hpp>

static Pair<Vector2f, Vector2f> getTextureUVs(const Texture& tex, const Rectf& srcRect)
{
    const Vector2f texSize(tex.getSize());
    float uvWidth  = (srcRect.x + srcRect.width  - 0.01f) / texSize.x;
    float uvHeight = (srcRect.y + srcRect.height - 0.01f) / texSize.y;
    float uvX      = (srcRect.x + 0.02f) / texSize.x;
    float uvY      = (srcRect.y + 0.02f) / texSize.y;
    return { Vector2f(uvX, uvY), Vector2f(uvWidth, uvHeight) };
}

struct RenderState
{
    GPUVertexData*                  mesh;
    Shader*                         shader         = nullptr;
    FixedVector<Texture*, 4, false> textures;
    GLDrawMode                      drawMode       = GLDrawMode::Triangles;
    GLBlendMode                     srcBlendFactor = GLBlendMode::SrcAlpha;
    GLBlendMode                     dstBlendFactor = GLBlendMode::OneMinusSrcAlpha;
};

struct VideoMemoryInfo
{
    // https://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt

    // dedicated video memory, total size (in kb) of the GPU memory
    int32_t total            = 0;

    // total available memory, total size (in Kb) of the memory available for allocations
    int32_t totalAvailable   = 0;

    // current available dedicated video memory (in kb), currently unused GPU memory
    int32_t currentAvailable = 0;
};

struct DrawIndirectCmd
{
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
};

struct Renderer
{
protected:
    static inline size_t drawCalls = 0;
    static inline bool   isCreated = false;

public:
    inline static bool created() { return isCreated; }

    static void create()
    {
        if (created()) { return; }

        rendlog->info("Creating renderer...");

        setVSync(VSyncMode::Disabled);

        if (gl3wInit())
        { rendlog->critical("Failed to initialize GL!"); ASSERT(false); }
        if (!gl3wIsSupported(3, 3))
        { rendlog->critical("GL 3.3 not supported"); ASSERT(false); }

        rendlog->info("GL {}, GLSL {}",
                      std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))),
                      std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));

        rendlog->info("Available texture units: {}", getMaxTextureUnits());
        rendlog->info("Max texture size: {}", getMaxTextureSize());

        GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
        GL_CHECK(glDebugMessageCallback( defaultGLCallback, 0 ));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glEnable(GL_MULTISAMPLE));

        isCreated = true;
        rendlog->info("Renderer created");
    }

    static void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static void drawElements(RenderState& state)
    {
        const GLenum  srcBlendFactor = (GLenum)state.srcBlendFactor;
        const GLenum  dstBlendFactor = (GLenum)state.dstBlendFactor;
        const GLenum  drawMode       = static_cast<GLenum>(state.drawMode);
        const int32_t elementCount   = state.mesh->ebo.size();
        const GLenum  indiceType     = GL_UNSIGNED_INT;
        const void*   indicePtr      = NULL; // Use NULL because our buffer is on the GPU
        
        if (elementCount > 0 || !state.mesh->buffers.empty() && state.mesh->buffers[0].size() > 0)
        {
            ASSERTMSG(elementCount,                  "You forgot to add indices");
            ASSERTMSG(state.mesh->buffers[0].size(), "You forgot your vertex data");
        }

        state.  mesh->bind();
        state.shader->bind();

        GL_CHECK(glBlendFunc(srcBlendFactor, dstBlendFactor));
        GL_CHECK(glDrawElements(drawMode, elementCount, indiceType, indicePtr));

        state.mesh->unbind();

        ++drawCalls;
    }

    static void drawElementsInstanced(RenderState& state, uint32_t instanceCount)
    {
        const GLenum  srcBlendFactor = (GLenum)state.srcBlendFactor;
        const GLenum  dstBlendFactor = (GLenum)state.dstBlendFactor;
        const GLenum  drawMode       = static_cast<GLenum>(state.drawMode);
        const int32_t elementCount   = state.mesh->ebo.size();
        const GLenum  indiceType     = GL_UNSIGNED_INT;
        const void*   indicePtr      = NULL; // Use NULL because our buffer is on the GPU
        
        if (elementCount > 0 || !state.mesh->buffers.empty() && state.mesh->buffers[0].size() > 0)
        {
            ASSERTMSG(elementCount,                  "You forgot to add indices");
            ASSERTMSG(state.mesh->buffers[0].size(), "You forgot your vertex data");
        }

        state.  mesh->bind();
        state.shader->bind();

        GL_CHECK(glBlendFunc(srcBlendFactor, dstBlendFactor));
        GL_CHECK(glDrawElementsInstanced(drawMode, elementCount, indiceType, indicePtr, instanceCount));

        state.mesh->unbind();

        ++drawCalls;
    }

    static void setViewport(const Recti& vp)
    {
        glViewport(vp.x,     vp.y,
                   vp.width, vp.height);
    }

    static void setViewport(int x, int y, int width, int height)
    { setViewport(Recti(x, y, width, height)); }

    [[nodiscard]] [[maybe_unused]]
    static Vector2i getFramebufferSize() noexcept
    {
        int fbw, fbh;
        SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &fbw, &fbh);
        return { fbw, fbh };
    }

    [[nodiscard]] [[maybe_unused]]
    static inline size_t getDrawCount() { return drawCalls; }

    [[maybe_unused]]
    static inline void resetDrawCount() { drawCalls = 0; }

    static inline void setVSync(VSyncMode mode)
    { SDL_GL_SetSwapInterval(static_cast<int>(mode)); }

    [[nodiscard]] [[maybe_unused]]
    static inline VSyncMode getVSync()
    { return static_cast<VSyncMode>(SDL_GL_GetSwapInterval()); }

    static inline int32_t getMaxTextureUnits()
    {
        int maxTextureUnits;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits));
        return maxTextureUnits;
    }

    static inline int32_t getMaxTextureSize()
    {
        int maxTextureSize;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize));
        return maxTextureSize;
    }

    static inline VideoMemoryInfo getVideoMemoryInfo()
    {
        VideoMemoryInfo info;
        glGetIntegerv(0x9047, &info.total);

        // If the last call failed, we're not on an nvidia card. clear the error and return.
        if (glGetError() != GL_NO_ERROR)
        { return info; }

        glGetIntegerv(0x9048, &info.totalAvailable);
        glGetIntegerv(0x9049, &info.currentAvailable);
        return info;
    }
};
