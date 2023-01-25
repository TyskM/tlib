#pragma once

#define NOMINMAX

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include "GLHelpers.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "View.hpp"
#include "Font.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "SSBO.hpp"
#include "FrameBuffer.hpp"
#include "../Macros.hpp"
#include "../Logging.hpp"

std::shared_ptr<tlog::logger> rendlog = tlog::createConsoleLogger("Renderer");

template <typename T>
size_t getVecSizeInBytes(const std::vector<T>& vec)
{
    return vec.size() * sizeof(T);
}

int SDLEventFilterCB(void* userdata, SDL_Event* event);

enum class GLDrawMode : int
{
    Unknown                  = -1,
    POINTS                   = GL_POINTS,
    LINE_STRIP               = GL_LINE_STRIP,
    LINE_LOOP                = GL_LINE_LOOP,
    LINES                    = GL_LINES,
    LINE_STRIP_ADJACENCY     = GL_LINE_STRIP_ADJACENCY,
    LINES_ADJACENCY          = GL_LINES_ADJACENCY,
    TRIANGLE_STRIP           = GL_TRIANGLE_STRIP,
    TRIANGLE_FAN             = GL_TRIANGLE_FAN,
    TRIANGLES                = GL_TRIANGLES,
    TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
    TRIANGLES_ADJACENCY      = GL_TRIANGLES_ADJACENCY,
    PATCHES                  = GL_PATCHES
};

enum class VSyncMode
{
    Disabled =  0,
    Enabled  =  1,
    Adaptive = -1
};

struct Renderer
{
    // Config

    // Read / Write
    // If true, renderer will use black bars to maintain views aspect ratio.
    // Also adjusts getMouseWorldPos() to return accurate coords.
    bool maintainAspect = false;       

    // Read / Write
    // Culls things drawn outside the view frustum.
    // It's super fast!! Probably no reason to turn it off.
    bool frustumCullingEnabled = true; 


    Window*       window = nullptr;         // Read only
    SDL_GLContext glContext = nullptr;      // Read only
    size_t        lastFrameDrawCalls = 0;   // Read only
    bool          begun = false;            // Read only
    View          view;                     // Read only
    glm::mat4     projection = glm::mat4(); // Read only
    Frustum       frustum;                  // Read only
    Shader        primShader;               // Read only
    VertexArray   linesVAO{NoCreate};       // Read only
    VertexBuffer  linesVBO{NoCreate};       // Read only
    size_t        debugDrawCalls = 0;       // Private

    // Caching
    mutable Rectf cachedVirtualViewport;    // Read only
    mutable bool  virtViewportDirty = true; // Read only

    // GPU Params
    int maxTextureUnits = 0; // Read only
    int maxTextureSize  = 0; // Read only

    Renderer() = default;
    ~Renderer() { reset(); }

    #pragma region Misc

    void create(Window& window)
    {
        this->window = &window;

        ASSERTMSG(WindowFlags::OpenGL & window.getFlags(),
            "Your window must be created with the WindowFlags::OpenGL flag!");

        glContext = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, glContext);
        SDL_GL_SetSwapInterval(0);

        if (gl3wInit())
        { rendlog->critical("Failed to initialize OpenGL!"); ASSERT(false); }
        if (!gl3wIsSupported(3, 3))
        { rendlog->critical("OpenGL 3.3 not supported"); ASSERT(false); }

        rendlog->info("OpenGL {}, GLSL {}",
                   std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))),
                   std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));

        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits));
        rendlog->info("Available texture units: {}", maxTextureUnits);

        GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize));
        rendlog->info("Max texture size: {}", maxTextureSize);

        GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
        GL_CHECK(glDebugMessageCallback( defaultGLCallback, 0 ));

        // Draw triangles in clockwise order
        GL_CHECK(glFrontFace(GL_CW));

        // Enable transparency
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GL_CHECK(glEnable(GL_MULTISAMPLE));

        // Primitives setup
        primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        initTextures();

        // Lines setup
        linesVAO.create();
        linesVBO.create();

        initText();

        SDL_AddEventWatch(SDLEventFilterCB, this);
        setView(getDefaultWindowView(window));
    }

    bool created() { return window != nullptr; }

    void reset()
    {
        if (!created()) return;
        SDL_GL_DeleteContext(glContext);
    }

    void begin()
    {
        ASSERTMSG(begun == false, "Forgot to call render()");
        begun = true;

        debugDrawCalls = 0;
        virtViewportDirty = true;

        // Draw black bars
        auto fbSize = window->getFramebufferSize();
        glViewport(0, 0, fbSize.x, fbSize.y);
        glDisable(GL_SCISSOR_TEST);
        clearColor(ColorRGBAf::black());

        resetViewport();
    }

    void render()
    {
        ASSERTMSG(begun == true, "Forgot to call begin()");
        begun = false;
        flushTextureBatch();
        flushLineBatch();

        lastFrameDrawCalls = debugDrawCalls;
        debugDrawCalls = 0;
    }

    // Usually only used internally
    // You probably want getView().bounds
    Rectf getVirtualViewportRect() const
    {
        Vector2f winSize = Vector2f(window->getFramebufferSize());

        if (!virtViewportDirty) { return cachedVirtualViewport; }
        Rectf vp;

        if (maintainAspect)
        {
            float targetAspectRatio = view.bounds.width / view.bounds.height;
            float width = winSize.x ;
            float height = width / targetAspectRatio + 0.5f;

            if (height > winSize.y )
            {
                height = winSize.y;
                width = height * targetAspectRatio + 0.5f;
            }

            float vp_x = (winSize.x / 2) - (width  / 2);
            float vp_y = (winSize.y / 2) - (height / 2);

            vp = Rectf( vp_x, winSize.y - (vp_y + height), width, height );
        }
        else
        {
            vp = Rectf(0, 0, view.bounds.width, view.bounds.height);
        }

        cachedVirtualViewport = vp;
        virtViewportDirty = false;
        return vp;
    }

    Vector2f getMouseLocalPos()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return Vector2f(static_cast<float>(x), static_cast<float>(y));
    }

    Vector2f getMouseWorldPos()
    {
        auto rect = getVirtualViewportRect();
        Vector2f mpos = getMouseLocalPos();
        mpos -= { rect.x, rect.y };

        glm::mat4 mat = view.getMatrix();
        mat = glm::inverse(mat);
        glm::vec3 ndc = glm::vec3(mpos.x / rect.width, 1.0 - mpos.y / rect.height, 0) * 2.f - 1.f;
        glm::vec4 worldPosition = mat * glm::vec4(ndc, 1);

        return { worldPosition.x, worldPosition.y };
    }

    void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // TODO: Use dirty flag instead
    void setView(const View& newView)
    {
        if (view == newView) { return; }

        view = newView;
        projection = view.getMatrix();
        frustum = Frustum(projection);

        spriteShader.bind();
        spriteShader.setMat4f("projection", projection);
        fastSpriteShader.bind();
        fastSpriteShader.setMat4f("projection", projection);
        primShader.bind();
        primShader.setMat4f("projection", projection);
        sdfFontShader.bind();
        sdfFontShader.setMat4f("projection", projection);
    }

    View getView() const { return view; }

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = Vector2f(win.getFramebufferSize());
        return View( 0, 0, winSize.x, winSize.y );
    }

    inline View getDefaultView() { return getDefaultWindowView(*window); }

    inline void setVSync(VSyncMode mode)
    { SDL_GL_SetSwapInterval(static_cast<int>(mode)); }

    inline VSyncMode getVSync()
    { return static_cast<VSyncMode>(SDL_GL_GetSwapInterval()); }

    void resetViewport()
    {
        auto rect = getVirtualViewportRect();
        auto fb   = window->getFramebufferSize();

        // OpenGL considers X=0, Y=0 the Lower left Corner of the Screen
        GLsizei x = static_cast<GLsizei>(rect.x);
        GLsizei fixedY = static_cast<GLsizei>( -rect.y + (fb.y - rect.height) );
        GLsizei width  = static_cast<GLsizei>(rect.width);
        GLsizei height = static_cast<GLsizei>(rect.height);

        glViewport(x, fixedY, width, height);
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, fixedY, width, height);
    }

    #pragma endregion

    #pragma region Slow Textures

    Shader       spriteShader;
    VertexArray  spriteVAO{NoCreate};
    VertexBuffer spriteVBO{NoCreate};

    void initTextures()
    {
        #pragma region Shader Source
        // Sprite setup
        spriteShader.create(
        // VERT
  R"""( #version 460 core
        //layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
        layout (location = 0) in vec2 vertex; // <vec2 texCoords>
        out vec2 TexCoords;

        vec2 pos[4] = vec2[4](vec2(0.0f, 1.0f),
                              vec2(1.0f, 1.0f),
                              vec2(0.0f, 0.0f),
                              vec2(1.0f, 0.0f));

        uniform mat4 model;
        uniform mat4 projection;

        void main()
        {
            TexCoords = vertex;
    
            gl_Position = projection * model * vec4(pos[gl_VertexID], 0.0, 1.0);
        } )""",
        // FRAG
        R"""(
        #version 460 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D image;
        uniform vec4 modulate;

        void main()
        {
            color = modulate * texture(image, TexCoords);
        } )""");
        #pragma endregion
        spriteShader.bind();
        spriteShader.setInt("image", 0);
        spriteShader.setVec4f("modulate", 1, 1, 1, 1);
        spriteShader.unbind();

        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        spriteVAO.create();
        spriteVBO.create();

        initFastTextures();
    }

    // Rot is in radians
    void drawTexture(Texture& tex, const Rectf& srcRect, const Rectf& dstRect,
                     const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 },
                     Shader* shader = nullptr, bool flipuvx = false, bool flipuvy = false)
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible(
            { dstRect.x, dstRect.y, 0 },
            { dstRect.x + dstRect.width, dstRect.y + dstRect.height, 0}))
        { return; }

        flushTextureBatch();
        flushLineBatch();
        
        if (shader == nullptr) { shader = &spriteShader; }
        shader->bind();

        glm::mat4 model = glm::mat4(1.0f);
        
        model = glm::translate(model, glm::vec3(dstRect.x, dstRect.y, 0.0f));

        if (rot != 0)
        {
            model = glm::translate(model, glm::vec3(0.5f * dstRect.width, 0.5f * dstRect.height, 0.0f));
            model = glm::rotate   (model, rot, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-0.5f * dstRect.width, -0.5f * dstRect.height, 0.0f));
        }

        model = glm::scale(model, glm::vec3(dstRect.width, dstRect.height, 1.f));

        shader->setMat4f("model", model);
        shader->setVec4f("modulate", color.r, color.g, color.b, color.a);

        const Vector2f texSize(tex.getSize());
        float uvWidth  = (srcRect.x + srcRect.width)  / texSize.x;
        float uvHeight = (srcRect.y + srcRect.height) / texSize.y;
        float uvX      = srcRect.x      / texSize.x;
        float uvY      = srcRect.y      / texSize.y;

        if (flipuvx)
        {
            auto tmp = uvX;
            uvX = uvWidth;
            uvWidth = tmp;
        }
        if (flipuvy)
        {
            auto tmp = uvY;
            uvY = uvHeight;
            uvHeight = tmp;
        }

        const float arr[] = { uvX,     uvHeight, // topleft
                              uvWidth, uvHeight, // topright
                              uvX,     uvY,      // bottom left
                              uvWidth, uvY };    // bottom right

        tex.bind();
        spriteVAO.bind();
        spriteVBO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(arr), arr, GL_DYNAMIC_DRAW));
        GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices));

        ++debugDrawCalls;
    }

    void drawTexture(Texture& tex, const Rectf& dstRect,
                     const float rot = 0, const ColorRGBAf& color ={ 1,1,1,1 },
                     Shader* shader = nullptr, bool flipuvx = false, bool flipuvy = false)
    {
        drawTexture(tex, Rectf(Vector2f(0, 0), Vector2f(tex.getSize())),
                    dstRect, rot, color, shader, flipuvx, flipuvy);
    }

    #pragma endregion

    #pragma region Fast Textures

    #pragma region Shader

    String glsl_version = "#version 460 core";
    String default_main_body = "defaultMain();";

    String uber_vert_src_unformatted = R"(
    `glsl_version`

    vec2 pos[4] = vec2[4](vec2(0.0f, 1.0f),
                          vec2(1.0f, 1.0f),
                          vec2(0.0f, 0.0f),
                          vec2(1.0f, 0.0f));

    struct Sprite
    {
        mat4 model;
        vec2 texCoords[4];
        vec4 color;
    };

    layout(std430, binding = 0) buffer data
    {
        Sprite sprites[`max_sprite_batch_size`];
    };

    out vec2 fragTexCoords;
    out vec4 fragColor;

    uniform mat4 projection;

    vec2 getVert()
    { return pos[gl_VertexID]; }

    mat4 getModel()
    { return sprites[gl_InstanceID].model; }

    vec2 getTexCoord()
    { return sprites[gl_InstanceID].texCoords[gl_VertexID]; }

    vec4 getColor()
    { return sprites[gl_InstanceID].color; }

    void setDefaultColor()
    { fragColor = getColor(); }

    void setDefaultTexCoord()
    { fragTexCoords = getTexCoord(); }

    void setDefaultVertPos()
    { gl_Position = projection * getModel() * vec4(getVert(), 0.0, 1.0); }

    void main()
    {
        setDefaultTexCoord();
        setDefaultColor();
        setDefaultVertPos();
    }
    )";
    
    String uber_frag_src_unformatted = R"(
    `glsl_version`
    in vec2 fragTexCoords;
    in vec4 fragColor;
    out vec4 color;

    uniform sampler2D image;
    uniform bool grayscale = false;

    vec2 getTexCoord()
    { return fragTexCoords; }

    vec4 getColor()
    { return fragColor; }

    void main()
    {
        if (grayscale) // GL_RED
        { color = vec4(getColor().xyz, 1.0) * vec4(1.0, 1.0, 1.0, texture(image, getTexCoord()).r); }

        else // RGBA
        { color = getColor() * texture(image, getTexCoord()); }
    }
    )";

    String formatShader(String str)
    {
        strhelp::replace(str, "{", "{{");
        strhelp::replace(str, "}", "}}");

        while (true)
        {
            if (strhelp::replaceFirst(str, "`", "{"))
            { strhelp::replaceFirst(str, "`", "}"); }
            else { break; }
        }

        auto fstr = fmt::format(fmt::runtime(str),
                                fmt::arg("glsl_version", glsl_version),
                                fmt::arg("max_sprite_batch_size", maxSpriteBatchSize));

        //rendlog->info(fstr);
        return fstr;
    }

    Shader initUberShader()
    {
        String vertSrc = formatShader(uber_vert_src_unformatted);
        String fragSrc = formatShader(uber_frag_src_unformatted);
        Shader retShader;
        retShader.create(vertSrc.c_str(), fragSrc.c_str());
        return retShader;
    }

    #pragma endregion

    static constexpr int maxSpriteBatchSize = 1024 * 8;

    struct FastSpriteInstance
    {
        glm::mat4 model;
        std::array<glm::vec2, 4> texCoords;
        glm::vec4 color;
    };

    int indices[6] ={ 0, 1, 2, 2, 1, 3 };
    std::array<FastSpriteInstance, maxSpriteBatchSize> spriteBatch ={};
    int currentSpriteBatchIndex = 0;
    Shader fastSpriteShader;
    SSBO texBatchDataSSBO;
    VertexArray dummyVAO; // Some drivers require that a VAO is bound when drawing, even though we don't need one.
    Texture* currentTexture = nullptr;
    bool     currentGrayscale = false;
    Shader*  currentShader = &fastSpriteShader;

    void initFastTextures()
    {
        fastSpriteShader = initUberShader();

        spriteBatch.fill(FastSpriteInstance());

        fastSpriteShader.bind();
        fastSpriteShader.setInt("image", 0);
        fastSpriteShader.unbind();

        texBatchDataSSBO.create();
        texBatchDataSSBO.bind();

        dummyVAO.create();
    }

    // Rot is in radians
    void drawTextureFast(Texture& tex, const Rectf& srcRect, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 },
                         bool flipuvx = false, bool flipuvy = false, bool grayscale = false,
                         Shader* shader = nullptr)
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible({ dstRect.x, dstRect.y, 0 }, {dstRect.x + dstRect.width, dstRect.y + dstRect.height, 0})) { return; }
        if (shader == nullptr) { shader = &fastSpriteShader; }
        if (&tex      != currentTexture || 
            grayscale != currentGrayscale ||
            shader    != currentShader) { flushTextureBatch(); }
        flushLineBatch();

        ASSERT(tex.created());
        ASSERT(srcRect.x >= 0 && srcRect.y >= 0);

        FastSpriteInstance& inst = spriteBatch[currentSpriteBatchIndex];

        inst.model = glm::mat4(1.0f);

        inst.model = glm::translate(inst.model, glm::vec3(dstRect.x, dstRect.y, 0.0f));

        if (rot != 0)
        {
            inst.model = glm::translate(inst.model, glm::vec3(0.5f * dstRect.width, 0.5f * dstRect.height, 0.0f));
            inst.model = glm::rotate   (inst.model, rot, glm::vec3(0.0f, 0.0f, 1.0f));
            inst.model = glm::translate(inst.model, glm::vec3(-0.5f * dstRect.width, -0.5f * dstRect.height, 0.0f));
        }

        inst.model = glm::scale(inst.model, glm::vec3(dstRect.width, dstRect.height, 1.f));
        
        const Vector2f texSize(tex.getSize());
        float normalWidth  = (srcRect.x + srcRect.width)  / texSize.x;
        float normalHeight = (srcRect.y + srcRect.height) / texSize.y;
        float normalX      = srcRect.x / texSize.x;
        float normalY      = srcRect.y / texSize.y;

        if (flipuvx)
        {
            auto tmp = normalX;
            normalX = normalWidth;
            normalWidth = tmp;
        }
        if (flipuvy)
        {
            auto tmp = normalY;
            normalY = normalHeight;
            normalHeight = tmp;
        }

        inst.texCoords[0] = { normalX,     normalHeight }; // topleft
        inst.texCoords[1] = { normalWidth, normalHeight }; // topright
        inst.texCoords[2] = { normalX,     normalY };      // bottom left
        inst.texCoords[3] = { normalWidth, normalY };      // bottom right

        inst.color = { color.r, color.g, color.b, color.a };

        ++currentSpriteBatchIndex;
        currentTexture = &tex;
        currentGrayscale = grayscale;
        currentShader = shader;

        if (currentSpriteBatchIndex == maxSpriteBatchSize) { flushTextureBatch(); }
    }

    void drawTextureFast(Texture& tex, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color ={ 1,1,1,1 },
                         bool flipuvx = false, bool flipuvy = false, bool grayscale = false,
                         Shader* shader = nullptr)
    {
        drawTextureFast(tex, Rectf{ Vector2f{0, 0}, Vector2f(tex.getSize()) }, dstRect, rot, color, flipuvx, flipuvy, grayscale, shader);
    }

    void drawTextureFast(SubTexture& subTex, const Rectf& dstRect,
                         float rot = 0, const ColorRGBAf& color ={ 1,1,1,1 },
                         bool flipuvx = false, bool flipuvy = false, bool grayscale = false,
                         Shader* shader = nullptr)
    {
        drawTextureFast(*subTex.texture, Rectf(subTex.rect), dstRect, rot, color, flipuvx, flipuvy, grayscale, shader);
    }

    // Call this to draw the batched textures early
    void flushTextureBatch()
    {
        if (currentTexture == nullptr || currentSpriteBatchIndex == 0) { return; }

        texBatchDataSSBO.bind();
        texBatchDataSSBO.bufferData(spriteBatch, currentSpriteBatchIndex, GL_DYNAMIC_DRAW);
        texBatchDataSSBO.setBufferBase(0);

        currentShader->bind();
        //fastSpriteShader.setBool("grayscale", currentGrayscale);
        currentTexture->bind();
        dummyVAO.bind();
        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, currentSpriteBatchIndex));

        texBatchDataSSBO.unbind();

        ++debugDrawCalls;
        currentSpriteBatchIndex = 0;
    }

    #pragma endregion

    #pragma region Frame Buffer

    void drawFrameBuffer(const FrameBuffer& fb, const Rectf& srcRect, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color ={ 1,1,1,1 }, bool flipuvx = false, bool flipuvy = true)
    {
        auto tmpView = getView();
        setView(getDefaultWindowView(*window));
        drawTexture(*fb.texture, srcRect, dstRect, rot, color, nullptr, flipuvx, flipuvy);
        setView(tmpView);
    }

    void drawFrameBuffer(const FrameBuffer& fb, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color ={ 1,1,1,1 }, bool flipuvx = false, bool flipuvy = true)
    {
        drawFrameBuffer(fb,
                        Rectf(Vector2f(0, 0), Vector2f(fb.texture->getSize())),
                        dstRect, rot, color, flipuvx, flipuvy);
    }

    #pragma endregion

    #pragma region Primitives

    #pragma region Shaders
    const char* PRIMITIVE_VERT_SHADER = R"""(
    #version 330 core
    layout (location = 0) in vec2 vertex;

    uniform mat4 projection;

    void main() 
    {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    }
    )""";

    const char* PRIMITIVE_FRAG_SHADER = R"""(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;

    void main()
    {
       FragColor = color;
    }
    )""";

    #pragma endregion

    std::vector<Vector2f> lineBatch;
    std::vector<GLuint>    lineBatchIndices;
    GLDrawMode lastMode  = GLDrawMode::Unknown;
    ColorRGBAf lastColor = { 1,1,1,1 };
    float      lastWidth = 1;
    static constexpr GLuint restartIndex = /*0xFFFF*/ std::numeric_limits<GLuint>::max();

    void flushLineBatch()
    {
        if (lineBatch.size() == 0) { return; }

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        primShader.bind();
        // Projection set in setView()
        primShader.setVec4f("color", lastColor.r, lastColor.g, lastColor.b, lastColor.a);

        linesVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, lineBatch.size() * sizeof(glm::vec2), lineBatch.data(), GL_DYNAMIC_DRAW));

        linesVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(2 * sizeof(float)), (void*)0));

        glLineWidth(lastWidth);
        GL_CHECK(glDrawElements(static_cast<GLenum>(lastMode), static_cast<GLsizei>(lineBatchIndices.size()), GL_UNSIGNED_INT, lineBatchIndices.data()));
        glDisable(GL_PRIMITIVE_RESTART);

        linesVBO.unbind();
        linesVAO.unbind();

        lineBatch.clear();
        lineBatchIndices.clear();
        ++debugDrawCalls;
    }

    // TODO: batch, it's super slow!!!
    template <typename Vector2fContainer>
    void drawLines(const Vector2fContainer& points, const ColorRGBAf& color = ColorRGBAf::white(),
                   float width = 1, GLDrawMode mode = GLDrawMode::LINE_STRIP)
    {
        flushTextureBatch();

        if (mode  != lastMode  ||
            color != lastColor ||
            width != lastWidth)
        {
            flushLineBatch();
        }

        lastMode  = mode;
        lastColor = color;
        lastWidth = width;

        for (auto& point : points)
        {
            lineBatchIndices.push_back(static_cast<GLuint>(lineBatch.size()));
            lineBatch.push_back(point);
        }
        lineBatchIndices.push_back(restartIndex);
    }

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { drawLines(std::vector{ start, end }, color, width); }

    void drawRect(float x, float y, float w, float h, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, float rot = 0.f)
    {
        std::vector<Vector2f> verts(4);
        verts[0] = Vector2f(x,     y);
        verts[1] = Vector2f(x + w, y);
        verts[2] = Vector2f(x + w, y + h);
        verts[3] = Vector2f(x,     y + h);

        if (rot != 0)
        {
            for (auto& v : verts)
            {
                v -= {x, y};
                v.rotate(rot);
                v += {x, y};
            }
        }

        drawLines(verts, color, 1, filled ? GLDrawMode::TRIANGLE_FAN : GLDrawMode::LINE_LOOP);
    }

    void drawRect(const Rectf& rect, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, float rot = 0.f)
    { drawRect(rect.x, rect.y, rect.width, rect.height, color, filled, rot); }

    void drawGrid(const Vector2f& start, const Vector2i& gridCount, Vector2f gridSize, const ColorRGBAf& color)
    {
        const float targetX = gridCount.x * gridSize.x + start.x;
        const float targetY = gridCount.y * gridSize.y + start.y;

        for (int x = 0; x <= gridCount.x; x++)
        {
            const float startx = x + start.x;
            drawLine(
                Vector2f{ startx * gridSize.x, 0 },
                Vector2f{ startx * gridSize.x, targetY },
                color
            );
        }

        for (int y = 0; y <= gridCount.y; y++)
        {
            const float starty = y + start.y;
            drawLine(
                Vector2f{ 0, starty * gridSize.y },
                Vector2f{ targetX, starty * gridSize.y },
                color
            );
        }
    }

    void drawCircle(const Vector2f& pos, const float rad,
                    const ColorRGBAf& color = ColorRGBAf::white(), const bool filled = false, const int segmentCount = 16)
    {
        const float theta = 3.1415926f * 2.f / static_cast<float>(segmentCount);
        const float tangetial_factor = tanf(theta);
        const float radial_factor = cosf(theta);

        float x = rad;
        float y = 0;

        std::vector<Vector2f> points;
        points.reserve(segmentCount);

        for (int i = 0; i < segmentCount; i++)
        {
            points.push_back(Vector2f{ x + pos.x, y + pos.y });

            float tx = -y;
            float ty = x;

            //add the tangential vector 

            x += tx * tangetial_factor;
            y += ty * tangetial_factor;

            //correct using the radial factor 

            x *= radial_factor;
            y *= radial_factor;
        }

        drawLines(points, color, 1.f, filled ? GLDrawMode::TRIANGLE_FAN : GLDrawMode::LINE_LOOP);
    }

    #pragma endregion

    #pragma region Text

    String sdf_font_frag_src_unformatted = R"(
    `glsl_version`
    in vec2 fragTexCoords;
    in vec4 fragColor;
    out vec4 color;

    uniform sampler2D image;

    vec2 getTexCoord()
    { return fragTexCoords; }

    vec4 getColor()
    { return fragColor; }

    const float width = 0.45;
    const float edge = 0.1;

    void main()
    {
        float distance = texture(image, getTexCoord()).r;
        float alpha = smoothstep(width, width + edge, distance);

        color = vec4(getColor().xyz, alpha);
    }
    )";

    Shader sdfFontShader;

    void initText()
    {
        auto vert = formatShader(uber_vert_src_unformatted);
        auto frag = formatShader(sdf_font_frag_src_unformatted);
        rendlog->info(vert);
        rendlog->info(frag);
        sdfFontShader.create(vert.c_str(), frag.c_str());
    }

    void drawTextSDF(const String& text, Font& font, const Vector2f& pos,
                     const ColorRGBAf& color = ColorRGBAf::white(), float scale = 1.f)
    {
        // TODO: save, set, then reset glDepthMask
        Vector2f currentPos = pos;
        for (auto& strchar : text)
        {
            // TODO: add default unknown character, and use that instead
            if (!font.characters.contains(strchar))
            { std::cout << "Font does not contain character \"" << strchar << "\"" << std::endl; continue; }

            Font::Character& ch = font.characters.at(strchar);
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            float xpos = (currentPos.x + ch.bearing.x);
            float ypos = (currentPos.y - h + (h - ch.bearing.y * scale));

            drawTextureFast(ch.texture, { xpos, ypos, w, h }, 0.f, color, false, false, true, &sdfFontShader);

            currentPos.x += (ch.advance >> 6) * scale;
        }
    }

    // TODO: print something on bad texture???
    // TODO: Draw from topleft
    // TODO: Put all glyphs into atlas
    // Pos will be the bottom left of the text
    void drawText(const String& text, Font& font, const Vector2f& pos,
                  const ColorRGBAf& color = ColorRGBAf::white(), float scale = 1.f)
    {
        Vector2f currentPos = pos;
        for (auto& strchar : text)
        {
            // TODO: add default unknown character, and use that instead
            if (!font.characters.contains(strchar))
            { std::cout << "Font does not contain character \"" << strchar << "\"" << std::endl; continue; }

            Font::Character& ch = font.characters.at(strchar);
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            float xpos = (currentPos.x + ch.bearing.x);
            float ypos = (currentPos.y - h + (h - ch.bearing.y * scale));
    
            drawTextureFast(ch.texture, { xpos, ypos, w, h }, 0.f, color, false, false, true);
    
            currentPos.x += (ch.advance >> 6) * scale;
        }
    }

    #pragma endregion
};

int SDLEventFilterCB(void* userdata, SDL_Event* event)
{
    Renderer* r = (Renderer*)userdata;
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    { /*r->_onWindowResized();*/ }
    return 1;
}