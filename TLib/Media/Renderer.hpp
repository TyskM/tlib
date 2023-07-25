#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/Shader.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Resource/Mesh.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Macros.hpp>

struct RenderState
{
    GLDrawMode  drawMode       = GLDrawMode::Triangles;
    GLBlendMode srcBlendFactor = GLBlendMode::SrcAlpha;
    GLBlendMode dstBlendFactor = GLBlendMode::OneMinusSrcAlpha;
};

struct Renderer
{
protected:
    static inline size_t drawCalls = 0;
    static inline bool   isCreated = false;

    static bool prepare(Shader& shader, Mesh& mesh, const RenderState& state)
    {
        shader.bind();

        if (!mesh.bind())
        {
            rendlog->error("Tried to draw a mesh that's in an invalid state");
            if (!mesh.validLayout())   rendlog->error("\tReason: Invalid layout. Did you forget to call Mesh::setLayout()?");
            if (!mesh.validVertices()) rendlog->error("\tReason: Invalid vertices. Did you forget to call Mesh::setData()?");
            return false;
        }

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

        int maxTextureUnits;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits));
        rendlog->info("Available texture units: {}", maxTextureUnits);

        int maxTextureSize;
        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize));
        rendlog->info("Max texture size: {}", maxTextureSize);

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

    static void draw(Shader& shader, Mesh& mesh, const RenderState& state = RenderState())
    {
        if (!prepare(shader, mesh, state)) { return; }

        const GLenum glmode = static_cast<GLenum>(state.drawMode);

        if (mesh.validIndices()) { GL_CHECK(glDrawElements(glmode, mesh.indiceCount(), GL_UNSIGNED_INT, (void*)0)); }
        else                     { GL_CHECK(glDrawArrays(glmode, 0, mesh.vertexCount())); }

        ++drawCalls;

        mesh.unbind();
    }

    static void drawInstanced(Shader& shader, Mesh& mesh, uint32_t count, const RenderState& state = RenderState())
    {
        if (!prepare(shader, mesh, state)) { return; }

        const GLenum glmode = static_cast<GLenum>(state.drawMode);

        if (mesh.validIndices()) { GL_CHECK(glDrawElementsInstanced(glmode, mesh.indiceCount(), GL_UNSIGNED_INT, (void*)0, count)); }
        else                     { GL_CHECK(glDrawArraysInstanced(glmode, 0, mesh.vertexCount(), count)); }

        ++drawCalls;

        mesh.unbind();
    }

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
};
