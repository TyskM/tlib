#pragma once

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include "GLHelpers.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Font.hpp"
#include "Shader.hpp"
#include "View.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "SSBO.hpp"
#include "FrameBuffer.hpp"
#include "../Macros.hpp"
#include "../Logging.hpp"

#include "Input.hpp"

std::shared_ptr<tlog::logger> rendlog = tlog::createConsoleLogger("Renderer");

#pragma region Shaders
const char* UBER_VERT_SHADER = R"(
    #version 460 core
    layout (location = 0) in vec2 vertex; // <vec2 texCoords>

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
        Sprite sprites    [)" STRING_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
    };

    uniform mat4 projection;
    out vec2 fragTexCoords;
    out vec4 fragColor;


    void defaultMain()
    {
        TexCoords = vertex;
        gl_Position = projection * model * vec4(pos[gl_VertexID], 0.0, 1.0);
    }

    void main() { defaultMain(); }
)";

const char* UBER_FRAG_SHADER = R"(
    #version 460 core
    in vec2 fragTexCoords;
    out vec4 color;

    uniform mat4 projection;
    uniform vec4 modulate;

    void defaultMain()
    {
        color = modulate * texture(image, fragTexCoords);
    }

    void main() { defaultMain(); }
)";

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

template <typename T>
size_t getVecSizeInBytes(const std::vector<T>& vec)
{
    return vec.size() * sizeof(T);
}

int SDLEventFilterCB(void* userdata, SDL_Event* event);

enum class GLDrawMode
{
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
    SDL_GLContext glContext;                // Read only
    size_t        lastFrameDrawCalls = 0;   // Read only
    bool          begun = false;            // Read only
    View          view;                     // Read only
    glm::mat4     projection;               // Read only
    Frustum       frustum;                  // Read only
    Shader        primShader;               // Read only
    Shader        textShader;               // Read only
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

        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        rendlog->info("Available texture units: {}", maxTextureUnits);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        rendlog->info("Max texture size: {}", maxTextureSize);

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback( defaultGLCallback, 0 );

        // Draw triangles in clockwise order
        glFrontFace(GL_CW);

        // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Primitives setup
        primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        textureInit();

        // Lines setup
        linesVAO.create();
        linesVBO.create();

        // Text setup
        textShader.create(TEXT_VERT_SHADER, TEXT_FRAG_SHADER);

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

        lastFrameDrawCalls = debugDrawCalls;
        debugDrawCalls = 0;
    }

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
        return Vector2f(x, y);
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
        textShader.bind();
        textShader.setMat4f("projection", projection);
    }

    View getView() const { return view; }

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = win.getFramebufferSize();
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
        int fixedY = -rect.y + (fb.y - rect.height);

        glViewport(rect.x, fixedY, rect.width, rect.height);
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.x, fixedY, rect.width, rect.height);
    }

    #pragma endregion

    #pragma region Texture

    Shader       spriteShader;
    VertexArray  spriteVAO{NoCreate};
    VertexBuffer spriteVBO{NoCreate};

    // Batched textures

    #ifndef MAX_SPRITE_BATCH_SIZE
    #define MAX_SPRITE_BATCH_SIZE 1024 * 8
    #endif

    struct FastSpriteInstance
    {
        glm::mat4 model;
        std::array<glm::vec2, 4> texCoords;
        glm::vec4 color;
    };

    int indices[6] = { 0, 1, 2, 2, 1, 3 };
    std::array<FastSpriteInstance, MAX_SPRITE_BATCH_SIZE> spriteBatch;
    int currentSpriteBatchIndex = 0;
    Shader fastSpriteShader;
    SSBO texBatchDataSSBO;
    Texture* currentTexture = nullptr;

    void textureInit()
    {
        #pragma region slow sprites
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
        spriteShader.bind();
        spriteShader.setInt("image", 0);
        spriteShader.setVec4f("modulate", 1, 1, 1, 1);
        spriteShader.unbind();

        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        spriteVAO.create();
        spriteVBO.create();

        #pragma endregion

        // Fast sprite setup
        #pragma region Shader Source
        fastSpriteShader.create(
            // VERTEX
            R"(
            #version 460 core

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
                Sprite sprites    [)" STRING_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
            };

            out vec2 fragTexCoords;
            out vec4 fragColor;

            
            uniform mat4 projection;

            void main()
            {
                Sprite sprite = sprites[gl_InstanceID];
                int vert = gl_VertexID;

                fragTexCoords = sprite.texCoords[vert];

                fragColor = sprite.color;
    
                gl_Position = projection * sprite.model * vec4(pos[vert], 0.0, 1.0);
            }
            )",
            // FRAG
            R"(
            #version 460 core
            in vec2 fragTexCoords;
            in vec4 fragColor;
            out vec4 color;

            uniform sampler2D image;

            void main()
            {
                color = fragColor * texture(image, fragTexCoords);
                //color = vec4(1, 0, 1, 1);
            }
            )"
        );
        #pragma endregion

        spriteBatch.fill(FastSpriteInstance());

        fastSpriteShader.bind();
        fastSpriteShader.setInt("image", 0);
        fastSpriteShader.unbind();

        texBatchDataSSBO.create();
        texBatchDataSSBO.bind();
    }

    void drawTexture(Texture& tex, const Rectf& srcRect, const Rectf& dstRect,
                     const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 },
                     Shader* shader = nullptr, bool flipuvx = false, bool flipuvy = false)
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible(
            { dstRect.x, dstRect.y, 0 },
            { dstRect.x + dstRect.width, dstRect.y + dstRect.height, 0}))
        { return; }

        flushTextureBatch();
        
        if (shader == nullptr) { shader = &spriteShader; }
        shader->bind();

        glm::mat4 model = glm::mat4(1.0f);
        
        model = glm::translate(model, glm::vec3(dstRect.x, dstRect.y, 0.0f));

        if (rot != 0)
        {
            model = glm::translate(model, glm::vec3(0.5f * dstRect.width, 0.5f * dstRect.height, 0.0f));
            model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
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

    void drawTextureFast(Texture& tex, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool flipuvx = false, bool flipuvy = false)
    {
        drawTextureFast(tex, Rectf{ Vector2f{0, 0}, Vector2f(tex.getSize()) }, dstRect, rot, color, flipuvx, flipuvy);
    }

    void drawTextureFast(SubTexture& subTex, const Rectf& dstRect,
                         float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool flipuvx = false, bool flipuvy = false)
    {
        drawTextureFast(*subTex.texture, Rectf(subTex.rect), dstRect, rot, color, flipuvx, flipuvy);
    }

    void drawTextureFast(Texture& tex, const Rectf& srcRect, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool flipuvx = false, bool flipuvy = false)
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible({ dstRect.x, dstRect.y, 0 }, {dstRect.x + dstRect.width, dstRect.y + dstRect.height, 0})) { return; }
        if (&tex != currentTexture) { flushTextureBatch(); }

        FastSpriteInstance& inst = spriteBatch[currentSpriteBatchIndex];

        inst.model = glm::mat4(1.0f);

        inst.model = glm::translate(inst.model, glm::vec3(dstRect.x, dstRect.y, 0.0f));

        if (rot != 0)
        {
            inst.model = glm::translate(inst.model, glm::vec3(0.5f * dstRect.width, 0.5f * dstRect.height, 0.0f));
            inst.model = glm::rotate   (inst.model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
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

        if (currentSpriteBatchIndex == MAX_SPRITE_BATCH_SIZE) { flushTextureBatch(); }
    }

    // Call this to draw the batched textures early
    void flushTextureBatch()
    {
        if (currentTexture == nullptr || currentSpriteBatchIndex == 0) { return; }

        texBatchDataSSBO.bind();
        texBatchDataSSBO.bufferData(spriteBatch, currentSpriteBatchIndex, GL_DYNAMIC_DRAW);
        texBatchDataSSBO.setBufferBase(0);

        fastSpriteShader.bind();
        currentTexture->bind();
        VertexArray::unbind();
        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices, currentSpriteBatchIndex));

        texBatchDataSSBO.unbind();

        ++debugDrawCalls;
        currentSpriteBatchIndex = 0;
    }

    void drawFrameBuffer(const FrameBuffer& fb, const Rectf& srcRect, const Rectf& dstRect,
                         const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool flipuvx = false, bool flipuvy = true)
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

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { drawLines(std::vector{ start, end }, color, width); }

    // TODO: batch, it's super slow!!!
    // It's a line loop
    template <typename Vector2fContainer>
    void drawLines(const Vector2fContainer& lines, const ColorRGBAf& color = ColorRGBAf::white(),
                   float width = 1, GLDrawMode mode = GLDrawMode::LINE_STRIP)
    {
        flushTextureBatch();

        primShader.bind();
        // Projection set in setView()
        primShader.setVec4f("color", color.r, color.g, color.b, color.a);

        linesVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vector2f), lines.data(), GL_DYNAMIC_DRAW));

        linesVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));

        glLineWidth(width);
        GL_CHECK(glDrawArrays((int)mode, 0, lines.size()));

        ++debugDrawCalls;
    }

    void drawRect(const Rectf& rect, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, float rot = 0.f)
    { drawRect(rect.x, rect.y, rect.width, rect.height, color, filled, rot); }

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
                    const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, const size_t segmentCount = 16)
    {
        const float theta = 3.1415926 * 2 / float(segmentCount);
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

    // Pos will be the bottom left of the text
    // Don't use
    void drawText(const std::string& text, Font& font, const Vector2f& pos,
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

            drawTexture(ch.texture, { xpos, ypos, w, h }, 0.f, color);

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