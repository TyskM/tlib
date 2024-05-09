#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/Shader.hpp>
#include <TLib/Media/GL/FrameBuffer.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Resource/GPUVertexData.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Containers/Span.hpp>

struct RenderState
{
    GLDrawMode  drawMode       = GLDrawMode::Triangles;
    GLBlendMode srcBlendFactor = GLBlendMode::SrcAlpha;
    GLBlendMode dstBlendFactor = GLBlendMode::OneMinusSrcAlpha;
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

    static bool prepare(Shader& shader, GPUVertexData& mesh, const RenderState& state)
    {
        if (!mesh.bind())
        {
            rendlog->error("Tried to draw a mesh that's in an invalid state");
            if (!mesh.validLayout())   rendlog->error("\tReason: Invalid layout. Did you forget to call Mesh::setLayout()?");
            if (!mesh.validVertices()) rendlog->error("\tReason: Invalid vertices. Did you forget to call Mesh::setData()?");
            return false;
        }
        
        return prepare(shader, state);
    }

    static bool prepare(Shader& shader, const RenderState& state)
    {
        shader.bind();
        GL_CHECK(glBlendFunc(static_cast<GLenum>(state.srcBlendFactor), static_cast<GLenum>(state.dstBlendFactor)));
        return true;
    }

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

    static void draw(Shader& shader, GPUVertexData& mesh, const RenderState& state = RenderState())
    {
        if (!prepare(shader, mesh, state)) { return; }

        const GLenum glmode = static_cast<GLenum>(state.drawMode);

        if (mesh.validIndices()) { GL_CHECK(glDrawElements(glmode, mesh.indiceCount(), GL_UNSIGNED_INT, (void*)0)); }
        else                     { GL_CHECK(glDrawArrays(glmode, 0, mesh.vertexCount())); }

        ++drawCalls;

        mesh.unbind(); // TODO: skip unbinds in release build
    }

    static void drawIndirect(
        Shader&                        shader,
        GPUVertexData&                 mesh,
        const Vector<DrawIndirectCmd>& cmds,
        const RenderState&             state = RenderState())
    {
        if (!prepare(shader, mesh, state)) { return; }
        ASSERT(mesh.validIndices());
        const GLenum glmode = static_cast<GLenum>(state.drawMode);

        GL_CHECK(glMultiDrawElementsIndirect(glmode, GL_UNSIGNED_INT, cmds.data(), cmds.size(), sizeof(cmds[0])));

        ++drawCalls;
    }

    static void drawInstanced(Shader& shader, GPUVertexData& mesh, uint32_t count, const RenderState& state = RenderState())
    {
        if (!prepare(shader, mesh, state)) { return; }

        const GLenum glmode = static_cast<GLenum>(state.drawMode);

        if (mesh.validIndices()) { GL_CHECK(glDrawElementsInstanced(glmode, mesh.indiceCount(), GL_UNSIGNED_INT, (void*)0, count)); }
        else                     { GL_CHECK(glDrawArraysInstanced(glmode, 0, mesh.vertexCount(), count)); }

        ++drawCalls;

        mesh.unbind();
    }

    static void drawIndices(Shader& shader, Span<uint32_t> indices, const RenderState& state = RenderState())
    {
        if (!prepare(shader, state)) { return; }
        const GLenum glmode = static_cast<GLenum>(state.drawMode);
        GL_CHECK(glDrawElements(glmode, indices.size(), GL_UNSIGNED_INT, indices.begin()));
    }

    static void setViewport(const Recti& vp, Vector2f targetSize = {-1,-1})
    {
        if (targetSize.x < 0)
        { targetSize = Vector2f(Renderer::getFramebufferSize()); }

        glViewport(vp.x,     targetSize.y - vp.y - vp.height, // because glViewport uses bottom left as origin
                   vp.width, vp.height);
    }

    static void setViewport(int x, int y, int width, int height, Vector2f targetSize ={-1,-1})
    { setViewport(Recti(x, y, width, height), targetSize); }

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
