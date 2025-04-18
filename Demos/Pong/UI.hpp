
#include <RmlUi/Core.h>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Resource/GPUVertexData.hpp>
#include <TLib/Media/Resource/Texture.hpp>

static std::shared_ptr<tlog::logger> rmluilog = tlog::createConsoleLogger("UI");

struct RmlRenderer : public Rml::RenderInterface
{
    #pragma region Public API

    void init()
    {
        GLubyte blankTextureData[1][1][4] = { { {255, 255, 255, 255} } };
        blankTexture.create();
        blankTexture.setData(blankTextureData, 1, 1);

        shader.create(shader_vert_main, shader_frag_color);
        shader.setInt("image", 0);
    }

    void beginFrame()
    {
        Renderer::clearColor();
        Renderer::setViewport(0, 0, 1280, 720);
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);

        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_CULL_FACE);

        // Set blending function for premultiplied alpha.
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, GLuint(-1));
        glStencilMask(GLuint(-1));
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glDisable(GL_DEPTH_TEST);
    }

    void renderFrame()
    {
        
    }

    #pragma endregion

    const char* shader_vert_main = R"(
    #version 330 core
    uniform vec2 _translate;
    uniform mat4 _transform;

    in vec2 inPosition;
    in vec4 inColor0;
    in vec2 inTexCoord0;

    out vec2 fragTexCoords;
    out vec4 fragColor;

    void main()
    {
        fragTexCoords = inTexCoord0;

        // Divide by 255 because RmlUi devs are using ints for colors for some reason
        fragColor     = inColor0 / 255;

        vec2 translatedPos = inPosition + _translate;
        vec4 outPos        = _transform * vec4(translatedPos, 0.0, 1.0);

        gl_Position = outPos;
    }
    )";

    const char* shader_frag_color = R"(
    #version 330 core
    in  vec2 fragTexCoords;
    in  vec4 fragColor;
    out vec4 outColor;

    uniform sampler2D image;

    void main()
    {
        outColor = fragColor * texture(image, vec2(fragTexCoords.x, fragTexCoords.y));
    }
    )";

    Shader  shader;
    Texture blankTexture;

    static constexpr Rml::TextureHandle TexturePostprocess = Rml::TextureHandle(-2);

    /// Called by RmlUi when it wants to compile geometry to be rendered later.
    /// @param[in] vertices The geometry's vertex data.
    /// @param[in] indices The geometry's index data.
    /// @return An application-specified handle to the geometry, or zero if it could not be compiled.
    /// @lifetime The pointed-to vertex and index data are guaranteed to be valid and immutable until ReleaseGeometry()
    /// is called with the geometry handle returned here.
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override
    {
        GPUVertexData* geometry = new GPUVertexData();

        TLib::Layout layout = { { 2, GLType::Float }, { 4, GLType::UByte }, { 2, GLType::Float } };
        geometry->setLayout(layout);
        geometry->setData<Rml::Span<const Rml::Vertex>, const Rml::Vertex>(vertices);
        geometry->setIndices<Rml::Span<const int>, const int>(indices);

        return (Rml::CompiledGeometryHandle)geometry;
    }

    /// Called by RmlUi when it wants to render geometry.
    /// @param[in] geometry The geometry to render.
    /// @param[in] translation The translation to apply to the geometry.
    /// @param[in] texture The texture to be applied to the geometry, or zero if the geometry is untextured.
    void RenderGeometry(Rml::CompiledGeometryHandle geometryHandle, Rml::Vector2f translation, Rml::TextureHandle rmlTexture) override
    {
        GPUVertexData* geometry = (GPUVertexData*)geometryHandle;

        if (rmlTexture == TexturePostprocess)
        {
            // Do nothing.
        }
        if (rmlTexture)
        {
            Texture* texture = (Texture*)rmlTexture;
            texture->bind();
        }
        else
        {
            blankTexture.bind();
        }

        View view;
        view.size   = { 1280, 720 };
        view.center = view.size/2.f;
        shader.setMat4f("_transform", view.getMatrix(true));
        shader.setVec2f("_translate", translation.x, translation.y);

        RenderState rs;
        rs.drawMode = GLDrawMode::Triangles;
        rs.srcBlendFactor = GLBlendMode::One;
        rs.dstBlendFactor = GLBlendMode::OneMinusSrcAlpha;
        Renderer::draw(shader, *geometry, rs);
    }

    /// Called by RmlUi when it wants to release geometry.
    /// @param[in] geometry The geometry to release.
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometryHandle) override
    {
        GPUVertexData* geometry = (GPUVertexData*)geometryHandle;
        delete geometry;
    }

    /// Called by RmlUi when a texture is required by the library.
    /// @param[out] texture_dimensions The dimensions of the loaded texture, which must be set by the application.
    /// @param[in] source The application-defined image source, joined with the path of the referencing document.
    /// @return An application-specified handle identifying the texture, or zero if it could not be loaded.
    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
    {
        rmluilog->warn("LoadTexture: Not implemented");
        return 0;
    }

    /// Called by RmlUi when a texture is required to be generated from a sequence of pixels in memory.
    /// @param[in] source The raw texture data. Each pixel is made up of four 8-bit values, red, green, blue, and premultiplied alpha, in that order.
    /// @param[in] source_dimensions The dimensions, in pixels, of the source data.
    /// @return An application-specified handle identifying the texture, or zero if it could not be generated.
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override
    {
        Texture* texture = new Texture();
        texture->setData(source.data(), source_dimensions.x, source_dimensions.y,
            TexPixelFormats::RGBA, TexInternalFormats::RGBA, TexPixelType::UnsignedByte);
        return (Rml::TextureHandle)texture;
    }

    /// Called by RmlUi when a loaded or generated texture is no longer required.
    /// @param[in] texture The texture handle to release.
    void ReleaseTexture(Rml::TextureHandle rmlTexture) override
    {
        Texture* texture = (Texture*)rmlTexture;
        delete texture;
    }

    /// Called by RmlUi when it wants to enable or disable scissoring to clip content.
    /// @param[in] enable True if scissoring is to enabled, false if it is to be disabled.
    void EnableScissorRegion(bool enable) override
    {
        if (enable)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
    }

    /// Called by RmlUi when it wants to change the scissor region.
    /// @param[in] region The region to be rendered. All pixels outside this region should be clipped.
    /// @note The region should be applied in window coordinates regardless of any active transform.
    void SetScissorRegion(Rml::Rectanglei region) override
    {
        glScissor(region.Left(), region.Top(), region.Width(), region.Height());
    }

    /** @name Optional functions for advanced rendering features. */

    /// Called by RmlUi when it wants to enable or disable the clip mask.
    /// @param[in] enable True if the clip mask is to be enabled, false if it is to be disabled.
    void EnableClipMask(bool enable) override
    {
        rmluilog->warn("EnableClipMask: Not implemented");
    }

    /// Called by RmlUi when it wants to set or modify the contents of the clip mask.
    /// @param[in] operation Describes how the geometry should affect the clip mask.
    /// @param[in] geometry The compiled geometry to render.
    /// @param[in] translation The translation to apply to the geometry.
    /// @note When enabled, the clip mask should hide any rendered contents outside the area of the mask.
    /// @note The clip mask applies exclusively to all other functions that render with a geometry handle, in addition
    /// to the layer compositing function while rendering to its destination.
    void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override
    {
        rmluilog->warn("RenderToClipMask: Not implemented");
    }

    /// Called by RmlUi when it wants the renderer to use a new transform matrix.
    /// @param[in] transform The new transform to apply, or nullptr if no transform applies to the current element.
    /// @note When nullptr is submitted, the renderer should use an identity transform matrix or otherwise omit the
    /// multiplication with the transform.
    /// @note The transform applies to all functions that render with a geometry handle, and only those.
    void SetTransform(const Rml::Matrix4f* transform) override
    {
        rmluilog->warn("SetTransform: Not implemented");
    }

    /// Called by RmlUi when it wants to push a new layer onto the render stack, setting it as the new render target.
    /// @return An application-specified handle representing the new layer. The value 'zero' is reserved for the initial base layer.
    /// @note The new layer should be initialized to transparent black within the current scissor region.
    Rml::LayerHandle PushLayer() override
    {
        rmluilog->warn("PushLayer: Not implemented");
        return 0;
    }

    /// Composite two layers with the given blend mode and apply filters.
    /// @param[in] source The source layer.
    /// @param[in] destination The destination layer.
    /// @param[in] blend_mode The mode used to blend the source layer onto the destination layer.
    /// @param[in] filters A list of compiled filters which should be applied before blending.
    /// @note Source and destination can reference the same layer.
    void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode blend_mode, Rml::Span<const Rml::CompiledFilterHandle> filters) override
    {
        rmluilog->warn("CompositeLayers: Not implemented");
    }

    /// Called by RmlUi when it wants to pop the render layer stack, setting the new top layer as the render target.
    void PopLayer() override
    {
        rmluilog->warn("PopLayer: Not implemented");
    }

    /// Called by RmlUi when it wants to store the current layer as a new texture to be rendered later with geometry.
    /// @return An application-specified handle to the new texture.
    /// @note The texture should be extracted using the bounds defined by the active scissor region, thereby matching its size.
    Rml::TextureHandle SaveLayerAsTexture() override
    {
        rmluilog->warn("SaveLayerAsTexture: Not implemented");
        return 0;
    }

    /// Called by RmlUi when it wants to store the current layer as a mask image, to be applied later as a filter.
    /// @return An application-specified handle to a new filter representing the stored mask image.
    Rml::CompiledFilterHandle SaveLayerAsMaskImage() override
    {
        rmluilog->warn("SaveLayerAsMaskImage: Not implemented");
        return 0;
    }

    /// Called by RmlUi when it wants to compile a new filter.
    /// @param[in] name The name of the filter.
    /// @param[in] parameters The list of name-value parameters specified for the filter.
    /// @return An application-specified handle representing the compiled filter.
    Rml::CompiledFilterHandle CompileFilter(const String& name, const Rml::Dictionary& parameters) override
    {
        rmluilog->warn("CompileFilter: Not implemented");
        return 0;
    }

    /// Called by RmlUi when it no longer needs a previously compiled filter.
    /// @param[in] filter The handle to a previously compiled filter.
    void ReleaseFilter(Rml::CompiledFilterHandle filter) override
    {
        rmluilog->warn("ReleaseFilter: Not implemented");
    }

    /// Called by RmlUi when it wants to compile a new shader.
    /// @param[in] name The name of the shader.
    /// @param[in] parameters The list of name-value parameters specified for the filter.
    /// @return An application-specified handle representing the shader.
    Rml::CompiledShaderHandle CompileShader(const String& name, const Rml::Dictionary& parameters) override
    {
        rmluilog->warn("CompileShader: Not implemented");
        return 0;
    }

    /// Called by RmlUi when it wants to render geometry using the given shader.
    /// @param[in] shader The handle to a previously compiled shader.
    /// @param[in] geometry The handle to a previously compiled geometry.
    /// @param[in] translation The translation to apply to the geometry.
    /// @param[in] texture The texture to use when rendering the geometry, or zero for no texture.
    void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override
    {
        rmluilog->warn("RenderShader: Not implemented");
    }
    
    /// Called by RmlUi when it no longer needs a previously compiled shader.
    /// @param[in] shader The handle to a previously compiled shader.
    void ReleaseShader(Rml::CompiledShaderHandle shader) override
    {
        rmluilog->warn("ReleaseShader: Not implemented");
    }
};

struct RmlSystem : public Rml::SystemInterface
{
    /// Get the number of seconds elapsed since the start of the application.
    /// @return Elapsed time, in seconds.
    //double GetElapsedTime() override
    //{
    //    return 0;
    //}

    /// Translate the input string into the translated string.
    /// @param[out] translated Translated string ready for display.
    /// @param[in] input String as received from XML.
    /// @return Number of translations that occured.
    //int TranslateString(String& translated, const String& input) override
    //{
    //    return 0;
    //}

    /// Joins the path of an RML or RCSS file with the path of a resource specified within the file.
    /// @param[out] translated_path The joined path.
    /// @param[in] document_path The path of the source document (including the file name).
    /// @param[in] path The path of the resource specified in the document.
    //void JoinPath(String& translated_path, const String& document_path, const String& path) override
    //{
    //
    //}

    /// Log the specified message.
    /// @param[in] type Type of log message, ERROR, WARNING, etc.
    /// @param[in] message Message to log.
    /// @return True to continue execution, false to break into the debugger.
    bool LogMessage(Rml::Log::Type type, const String& message) override
    {
        switch (type)
        {
            case Rml::Log::LT_WARNING:
                rmluilog->warn(message);
            break;

            case Rml::Log::LT_ALWAYS:
            case Rml::Log::LT_INFO:
                rmluilog->info(message);
            break;

            case Rml::Log::LT_DEBUG:
                rmluilog->debug(message);
            break;

            case Rml::Log::LT_MAX:
            case Rml::Log::LT_ERROR:
            case Rml::Log::LT_ASSERT:
            default: break;
                rmluilog->error(message);
            break;
        }
        return true;
    }

    /// Set mouse cursor.
    /// @param[in] cursor_name Cursor name to activate.
    //void SetMouseCursor(const String& cursor_name) override
    //{
    //
    //}

    /// Set clipboard text.
    /// @param[in] text Text to apply to clipboard.
    //void SetClipboardText(const String& text) override
    //{
    //
    //}

    /// Get clipboard text.
    /// @param[out] text Retrieved text from clipboard.
    //void GetClipboardText(String& text) override
    //{
    //
    //}

    /// Activate keyboard (for touchscreen devices).
    /// @param[in] caret_position Position of the caret in absolute window coordinates.
    /// @param[in] line_height Height of the current line being edited.
    //void ActivateKeyboard(Rml::Vector2f caret_position, float line_height) override
    //{
    //
    //}

    /// Deactivate keyboard (for touchscreen devices).
    //void DeactivateKeyboard() override
    //{
    //
    //}
};

struct RMLUI
{

};

void UItest()
{
    
}