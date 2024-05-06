
// TODO: Finish model example
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/RNG.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Containers/Span.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include "Common.hpp"

#include <TLib/Media/Resource/MeshData.hpp>
#include <TLib/Embed/Embed.hpp>
#include <TLib/Containers/Stack.hpp>

#define flecs_STATIC
#include <flecs.h>

#define PX_PHYSX_STATIC_LIB
#include <physx/PxPhysics.h>
#include <physx/PxPhysicsAPI.h>
#include <physx/foundation/PxErrorCallback.h>
#include <physx/foundation/PxFoundation.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/characterkinematic/PxCapsuleController.h>

static glm::vec3 toGlm(const Vector3f& v) { return { v.x, v.y, v.z }; };

#pragma region Physics

using namespace physx;

class TLibPhysicsAllocatorCallback : public physx::PxAllocatorCallback
{
public:
    virtual ~TLibPhysicsAllocatorCallback() {}
    virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
    { return mi_malloc(size); }
    virtual void deallocate(void* ptr)
    { return mi_free(ptr); }
};

class TLibPhysicsErrorCallback : public physx::PxErrorCallback
{
public:
    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
        tlog::error("PhysX error: File: {} Line: \n{}", file, line, message);
    }
};

static inline TLibPhysicsAllocatorCallback physicsDefaultAllocatorCallback;
static inline TLibPhysicsErrorCallback     physicsDefaultErrorCallback;
static inline physx::PxFoundation*         foundation = nullptr;
static void initPhysics()
{
    ASSERT(!foundation); // Foundation already exists
    foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
                                    physicsDefaultAllocatorCallback,
                                    physicsDefaultErrorCallback);
    ASSERT(foundation); // Failed to create foundation
}

static void shutdownPhysics()
{
    ASSERT(foundation);
    if (foundation)
        foundation->release();
}

PxPhysics* createPhysics(PxTolerancesScale& scale)
{
    bool recordMemoryAllocations = true;

    auto mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation,
        scale, recordMemoryAllocations, NULL);
    ASSERT(mPhysics);

    if (!PxInitExtensions(*mPhysics, NULL))
    { tlog::error("PxInitExtensions failed"); ASSERT(false); }

    return mPhysics;
}

PxScene* createPhysicsScene(PxPhysics* phys)
{
    PxSceneDesc desc(phys->getTolerancesScale());
    desc.filterShader  = PxDefaultSimulationFilterShader;
    desc.gravity       ={ 0.f, -20.f, 0.f };
    desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
    ASSERT(desc.isValid());
    PxScene* physicsScene = phys->createScene(desc);
    ASSERT(physicsScene);
    return physicsScene;
}

#pragma endregion

struct Camera3D
{
    enum class ViewMode
    {
        Perspective,
        Orthographic
    };

    float     znear    = 0.02f;
    float     zfar     = 3000.f;
    float     fov      = 110.f;
    Vector3f  pos      = { 0.0f, 0.0f, 0.0f };
    Quat      rot;
    ViewMode  viewmode = ViewMode::Perspective;

    inline void setPos(const Vector3f& posv) { this->pos ={ posv.x, posv.y, posv.z }; }
    [[nodiscard]] inline Vector3f getPos() const { return Vector3f(pos); }

    void lookAt(const Vector3f& target,
                const Vector3f& up    = Vector3f::up(),
                const Vector3f& altUp = Vector3f::backward())
    {
        rot = Quat::safeLookAt(pos, target, up, altUp);
    }

    [[nodiscard]]
    Mat4f getViewMatrix() const
    {
        Mat4f rotate = rot.toMat4f();
        Mat4f translate(1.0f);
        translate = translate.translate(-pos);
        return rotate * translate;
    }

    [[nodiscard]]
    Mat4f getPerspectiveMatrix(Vector2f size = Vector2f(Renderer::getFramebufferSize())) const
    {
        switch (viewmode)
        {
            case ViewMode::Perspective:
                return glm::perspective(glm::radians(fov), size.x / size.y, znear, zfar); break;
            case ViewMode::Orthographic:
            {
                Vector2f halfSize = size/2.f;
                return glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, znear, zfar); break;
            }
            default: return Mat4f(); break;
        }
    }
};

/* DONE: Optimize VAO usage. One per model? // Done, thanks aiProcess_OptimizeGraph

Use single global VAO, VBO, and EBO to avoid the overhead of switching them.

global VertexArray;  // Layout
global VertexBuffer; // Vertices
global ElementBuffer // Indices

size_t vboIndex, vboSize;
size_t eboIndex, eboSize;

*/
// Add lighting
// Support materials
struct Mesh : NonAssignable
{
private:
    struct MeshTexture
    {
        TextureType   type = TextureType::Diffuse;
        UPtr<Texture> texture;
    };

    struct Material
    {
        Array<UPtr<Texture>, (size_t)TextureType::Count> textures;
    };

    struct SubMesh
    {
        UPtr<GPUVertexData> vertices;
        Material            material;
    };

    Vector<SubMesh> meshes;

public:
    Vector<SubMesh>& getMeshes()
    { return meshes; }

    bool loadFromMemory(const MeshData& cpuMesh)
    {
        for (auto& cpuSubMesh : cpuMesh.subMeshes)
        {
            auto& gpuSubMesh = meshes.emplace_back();
            gpuSubMesh.vertices  = makeUnique<GPUVertexData>();
            gpuSubMesh.vertices->setLayout({ Layout::Vec3f(), Layout::Vec3f(), Layout::Vec2f() });
            gpuSubMesh.vertices->setData(cpuSubMesh.vertices);
            gpuSubMesh.vertices->setIndices(cpuSubMesh.indices);

            int32_t i = 0;
            for (auto& cpuTexture : cpuSubMesh.material.textures)
            {
                ASSERT(cpuTexture); // Textures should never be NULL, worst case they are 1x1 pure white/black/gray
                auto& gpuTexture = gpuSubMesh.material.textures[i];
                gpuTexture = makeUnique<Texture>();
                gpuTexture->setData(*cpuTexture);
                gpuTexture->setUVMode(UVMode::Repeat);
                gpuTexture->setFilter(TextureMinFilter::LinearMipmapLinear, TextureMagFilter::Linear);
                gpuTexture->generateMipmaps();
                ++i;
            }
        }

        return true;
    }

    // Returns false on failure
    bool loadFromFile(const Path& path)
    {
        MeshData meshData;
        bool success = meshData.loadFromFile(path);
        if (!success) { tlog::error("Model::loadFromFile: Failed to load mesh"); return false; }
        ASSERT(success);

        return loadFromMemory(meshData);
    }

    void reset()
    {
        meshes.clear();
    }
};

struct Transform3D
{
    Vector3f  pos;
    Vector3f  scale = { 1, 1, 1 };
    Quat      rot     { glm::vec3(0.f, 0.f, 0.f) };

    Transform3D() = default;
    Transform3D(const Vector3f& pos, const Vector3f& scale = Vector3f(1.f), const Quat& rot = Quat()) : pos{pos}, scale{scale}, rot{rot} { }

    Mat4f getMatrix() const
    {
        // Scale, rotate, translate
        Mat4f mat(1.f);
        mat  = mat.scale(scale);
        mat *= rot.toMat4f();
        mat  = mat.translate(pos);
        return mat;
    }

    void rotate(float angle, const Vector3f& axis)
    {
        rot *= Quat(angle, axis);
    }
};

class Renderer3D
{
public:
    static inline bool useMSAA      = true;  // Read / Write. Creates ugly artifacts (sometimes), find post processing solution.

    struct ModelDrawCmd
    {
        Mesh*       model;
        Transform3D transform;
    };

    struct PrimitiveDrawCmd
    {
        GLDrawMode drawMode;
        Shader*    shader  = &defaultPrimitiveShader;
        uint32_t   posIndex, posSize; // Index and size for posAndCoords
        uint32_t   indIndex, indSize; // Index and size for indices
        ColorRGBAf color;
    };

    using DrawCmd = Variant<PrimitiveDrawCmd, ModelDrawCmd>;

    struct PrimVertex
    {
        Vector3f   pos;
        ColorRGBAf color;
    };

    static inline Vector<PrimVertex> primitiveVerts;
    static inline Vector<uint32_t>   primitiveIndices;
    static inline GPUVertexData      primitiveMesh;
    static inline Shader             defaultPrimitiveShader;

    static constexpr GLuint restartIndex = std::numeric_limits<GLuint>::max();

    static inline Vector<DrawCmd> cmds;
    static inline Camera3D        camera;

    static inline Shader  shader3d;

    static inline FrameBuffer shadowFbo;
    static inline Texture     shadowTex;
    static inline uint32_t    shadowSize = 1024;
    static inline Shader      shadowShader;

    struct DirectionalLight
    {
        Vector3f  dir;
        ColorRGBf color;
        float     power = 1.f;
    };

    struct PointLight
    {
        Vector3f  pos;
        ColorRGBf color;
        float     power = 1.f;
    };

    struct SpotLight
    {
        Vector3f  pos;
        Vector3f  dir;
        ColorRGBf color;
        float     cosAngle      = 1.f;
        float     outerCosAngle = 0.2f;
        float     power         = 1.f;
    };

    static inline uint32_t                 maxDirectionalLights = 2;
    static inline Vector<DirectionalLight> directionalLights;

    static inline uint32_t           maxPointLights = 32;
    static inline Vector<PointLight> pointLights;

    static inline uint32_t          maxSpotLights = 32;
    static inline Vector<SpotLight> spotLights;

    static void drawMeshImmediate(GPUVertexData& mesh, Texture& tex, const Transform3D& transform)
    {
        tex.bind();
        shader3d.setMat4f("model", transform.getMatrix());
        Renderer::draw(shader3d, mesh);
    }

    static void flushPrimitives(Shader& shader, GLDrawMode drawMode, Span<uint32_t> indices)
    {
        primitiveMesh.bind();
        RenderState rs;
        rs.drawMode = drawMode;
        Renderer::drawIndices(shader, indices, rs);
    }

public:
    struct GLSLSource
    {
    private:
        String src;

    public:
        GLSLSource() = default;
        GLSLSource(const String& str) : src{str} { }

        void inject(const String& str)
        {
            size_t versionIndex = src.find("#version");
            if (versionIndex == src.size()) { tlog::error("Missing #version"); return; }
            size_t insertPoint = src.find('\n', versionIndex) + 1;
            src.insert(insertPoint, str + '\n');
        }

        String string() const
        { return src; }
    };

    static void setPBRShader(const String& vertsrc, const String& fragsrc)
    {
        GLSLSource vert(vertsrc); GLSLSource frag(fragsrc);
        frag.inject(fmt::format("#define maxDirectionalLights {}", maxDirectionalLights));
        frag.inject(fmt::format("#define maxPointLights       {}", maxPointLights));
        frag.inject(fmt::format("#define maxSpotLights        {}", maxSpotLights));

        if (!shader3d.create(vert.string(), frag.string()))
        {
            tlog::error("Failed to compile the following shader:");
            tlog::error("Vertex Shader:");
            tlog::error('\n' + vert.string());
            tlog::error("Fragment Shader:");
            tlog::error('\n' + frag.string());
        }
    }

    static bool created()
    {
        return shader3d.created();
    }

    static void setCamera(const Camera3D& camera)
    {
        // TODO: use one of those global uniform things
        Renderer3D::camera = camera;
        auto view = camera.getViewMatrix();

        auto proj = camera.getPerspectiveMatrix();
        shader3d.setMat4f("projection",    proj);
        shader3d.setMat4f("view",          view);
        shader3d.setVec3f("fragCameraPos", camera.pos);

        defaultPrimitiveShader.setMat4f("projection",    proj);
        defaultPrimitiveShader.setMat4f("view",          view);
    }

    static void drawModel(Mesh& model, const Transform3D& transform)
    {
        DrawCmd&      var = cmds.emplace_back();
        ModelDrawCmd& cmd = var.emplace<ModelDrawCmd>();
        cmd.model         = &model;
        cmd.transform     = transform;
    }

    static void drawLines(
        const Span<const Vector3f>& points,
        const ColorRGBAf&           color  = ColorRGBAf::white(),
        const GLDrawMode            mode   = GLDrawMode::LineStrip,
              Shader&               shader = defaultPrimitiveShader)
    {
        if (points.empty()) { return; }

        auto& var = cmds.emplace_back();
        auto& cmd = std::get<PrimitiveDrawCmd>(var);

        cmd.shader   = &shader;
        cmd.drawMode = mode;
        cmd.color    = color;

        cmd.indIndex = primitiveIndices.size();
        cmd.posIndex = primitiveVerts.size();
        
        cmd.indSize  = points.size();
        cmd.posSize  = points.size();

        //primitiveIndices.push_back(restartIndex);
        for (int i = 0; i < points.size(); i++)
        {
            const Vector3f& p = points[i];
            auto& vert = primitiveVerts.emplace_back();
            vert.pos   = p;
            vert.color = color;
            primitiveIndices.push_back(i);
        }
    }

    static void drawCube(const Vector3f& pos, const Vector3f& size = 1.f, const ColorRGBAf color = ColorRGBAf::white())
    {
        constexpr Vector3f vertices[8] =
        {   Vector3f(-1, -1, -1),
            Vector3f( 1, -1, -1),
            Vector3f( 1,  1, -1),
            Vector3f(-1,  1, -1),
            Vector3f(-1, -1,  1),
            Vector3f( 1, -1,  1),
            Vector3f( 1,  1,  1),
            Vector3f(-1,  1,  1) };

        constexpr int indices[36] =
        {   0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1  };

        Vector<Vector3f> lines;
        lines.reserve(36);
        for (uint32_t i = 0; i < 36; i++)
        { lines.push_back(vertices[indices[i]] * size + pos); }
        drawLines(lines, color, GLDrawMode::Triangles);
    }

    // Used for things like sun light
    static void addDirectionalLight(const Vector3f& direction, const ColorRGBf color, float power = 1.f)
    {
        DirectionalLight& light = directionalLights.emplace_back();
        light.dir   = direction;
        light.color = color;
        light.power = power;
    }

    static void addPointLight(const Vector3f& pos, const ColorRGBf& color, float power = 1.f)
    {
        PointLight& light = pointLights.emplace_back();
        light.pos   = pos;
        light.color = color;
        light.power = power;
    }

    static void addSpotLight(const Vector3f& pos, const Vector3f& dir, float angleDeg, float outerAngleDeg, const ColorRGBf& color, float power = 1.f)
    {
        SpotLight& light    = spotLights.emplace_back();
        light.pos           = pos;
        light.dir           = dir;
        light.cosAngle      = glm::cos(glm::radians(angleDeg));
        light.outerCosAngle = glm::cos(glm::radians(angleDeg + outerAngleDeg));
        light.color         = color;
        light.power         = power;
    }

    static void render()
    {
        if (cmds.empty()) { return; }

        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glFrontFace(GL_CCW));

        if (useMSAA) { GL_CHECK(glEnable(GL_MULTISAMPLE)); }
        else         { GL_CHECK(glDisable(GL_MULTISAMPLE)); }

        shader3d.setInt ("material.diffuse",   0);
        shader3d.setInt ("material.roughness", 1);
        shader3d.setInt ("material.metallic",  2);

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        // Upload lights
        {
            // Point Lights
            {
                int32_t pointLightCount = std::min<int32_t>(pointLights.size(), maxPointLights);
                shader3d.setInt("pointLightCount", pointLightCount);
                for (int32_t i = 0; i < pointLightCount; i++)
                {
                    shader3d.setVec3f(fmt::format("pointLights[{}].localPos",               i), pointLights[i].pos);
                    shader3d.setVec3f(fmt::format("pointLights[{}].light.color",            i), pointLights[i].color);
                    shader3d.setFloat(fmt::format("pointLights[{}].light.diffuseIntensity", i), pointLights[i].power);
                    shader3d.setFloat(fmt::format("pointLights[{}].light.ambientIntensity", i), 1.f);
                }
            }

            // Directional Lights
            {
                int32_t directionalLightCount = std::min<int32_t>(directionalLights.size(), maxDirectionalLights);
                shader3d.setInt("directionalLightCount", directionalLightCount);
                for (int32_t i = 0; i < directionalLightCount; i++)
                {
                    shader3d.setVec3f(fmt::format("directionalLights[{}].dir",                    i), directionalLights[i].dir);
                    shader3d.setVec3f(fmt::format("directionalLights[{}].light.color",            i), directionalLights[i].color);
                    shader3d.setFloat(fmt::format("directionalLights[{}].light.diffuseIntensity", i), directionalLights[i].power);
                    shader3d.setFloat(fmt::format("directionalLights[{}].light.ambientIntensity", i), 1.f);
                }
            }

            // Spot Lights
            {
                int32_t spotLightCount = std::min<int32_t>(spotLights.size(), maxSpotLights);
                shader3d.setInt("spotLightCount", spotLightCount);
                for (int32_t i = 0; i < spotLightCount; i++)
                {
                    shader3d.setVec3f(fmt::format("spotLights[{}].pos",                    i), spotLights[i].pos);
                    shader3d.setVec3f(fmt::format("spotLights[{}].dir",                    i), spotLights[i].dir);
                    shader3d.setFloat(fmt::format("spotLights[{}].cosAngle",               i), spotLights[i].cosAngle);
                    shader3d.setFloat(fmt::format("spotLights[{}].outerCosAngle",          i), spotLights[i].outerCosAngle);
                    shader3d.setVec3f(fmt::format("spotLights[{}].light.color",            i), spotLights[i].color);
                    shader3d.setFloat(fmt::format("spotLights[{}].light.diffuseIntensity", i), spotLights[i].power);
                    shader3d.setFloat(fmt::format("spotLights[{}].light.ambientIntensity", i), 1.f);
                }
            }
        }

        // Shadow pass
        {
            Camera3D shadowCamera;
            Vector3f dir    = directionalLights[0].dir.normalized();
            Vector3f pos    = Vector3f(camera.pos);
            Vector3f target = pos + dir;

            drawCube(target, 0.2f);

            // This works way better than glm::quatLookAt. what a world we live in.
            auto mat = glm::lookAt(pos.toGlm(), target.toGlm(), Vector3f::up().toGlm());
            shadowCamera.rot = glm::quat_cast(mat);

            // if dir.z is negative, this shadow map turns inside out???
            //shadowCamera.rot      = Quat::safeLookAt(Vector3f(), dir, Vector3f::up(), Vector3f::backward());
            shadowCamera.pos      = { pos.x, pos.y, pos.z };
            shadowCamera.viewmode = Camera3D::ViewMode::Orthographic;
            shadowCamera.zfar     =  80.f;
            shadowCamera.znear    = -80.f;

            ImGui::Begin("Shadow Camera");
            ImGui::Text(fmt::format("Dir  : {}", dir.toString()).c_str());
            ImGui::Text(fmt::format("Pos  : {}", pos.toString()).c_str());
            ImGui::Text(fmt::format("Tar  : {}", target.toString()).c_str());
            ImGui::Text(fmt::format("Quat : {}", shadowCamera.rot.toString()).c_str());
            ImGui::End();

            Mat4f lightProjection  = shadowCamera.getPerspectiveMatrix(256.f);
            Mat4f lightView        = shadowCamera.getViewMatrix();
            Mat4f lightSpaceMatrix = lightProjection * lightView;

            Renderer::setViewport(Recti(Vector2i(0), Vector2i(shadowSize, shadowSize)), Vector2f((float)shadowSize));
            shadowFbo.bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            for (auto& varCmd : cmds)
            {
                if (is<ModelDrawCmd>(varCmd))
                {
                    ModelDrawCmd& cmd = std::get<ModelDrawCmd>(varCmd);

                    for (auto& mesh : cmd.model->getMeshes())
                    {
                        shadowFbo.bind();
                        shadowShader.setMat4f("lightSpaceMatrix", lightSpaceMatrix);
                        shadowShader.setMat4f("model", cmd.transform.getMatrix());
                        Renderer::draw(shadowShader, *mesh.vertices);
                        shadowFbo.unbind();
                    }
                }
            }
            shadowFbo.unbind();
            Renderer2D::setView(Renderer2D::getView());
        }

        // PBR Pass
        for (auto& varCmd : cmds)
        {
            if (is<PrimitiveDrawCmd>(varCmd))
            {
                PrimitiveDrawCmd& cmd = std::get<PrimitiveDrawCmd>(varCmd);

                Span<PrimVertex> pos(primitiveVerts.begin()   + cmd.posIndex, cmd.posSize);
                Span<uint32_t>   ind(primitiveIndices.begin() + cmd.indIndex, cmd.indSize);

                primitiveMesh.setData   (pos, AccessType::Dynamic);
                primitiveMesh.setIndices(ind, AccessType::Dynamic);

                RenderState rs; rs.drawMode = cmd.drawMode;
                Renderer::draw(*cmd.shader, primitiveMesh, rs);
            }

            else if (is<ModelDrawCmd>(varCmd))
            {
                ModelDrawCmd& cmd = std::get<ModelDrawCmd>(varCmd);

                for (auto& mesh : cmd.model->getMeshes())
                {
                    mesh.material.textures[(int32_t)TextureType::Diffuse]  ->bind(0);
                    mesh.material.textures[(int32_t)TextureType::Roughness]->bind(1);
                    mesh.material.textures[(int32_t)TextureType::Metalness]->bind(2);

                    shader3d.setMat4f("model", cmd.transform.getMatrix());
                    Renderer::draw(shader3d, *mesh.vertices);
                }
            }
        }

        cmds.clear();
        primitiveVerts.clear();
        primitiveIndices.clear();

        pointLights.clear();
        directionalLights.clear();
        spotLights.clear();
    }

    static void create()
    {
        // Setup Primitive Rendering
        {
            primitiveMesh.setLayout({ Layout::Vec3f(), Layout::Vec4f() });

            defaultPrimitiveShader.create(
                myEmbeds.at("TLib/Embed/Shaders/3d_primitive.vert").asString(),
                myEmbeds.at("TLib/Embed/Shaders/3d_primitive.frag").asString());

            defaultPrimitiveShader.setMat4f("model", glm::mat4(1));
            primitiveVerts.reserve(1024 * 2);
            primitiveIndices.reserve(1024 * 2);
        }

        // Setup PBR Rendering
        {
            setPBRShader(myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString(),
                         myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString());
        }

        // Setup shadows
        {
            shadowTex.create();
            shadowTex.setData(NULL, shadowSize, shadowSize, TexPixelFormats::DEPTH_COMPONENT, TexInternalFormats::DEPTH_COMPONENT);
            shadowTex.setFilter(TextureFiltering::Nearest);
            shadowTex.setUVMode(UVMode::Repeat);
            GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            shadowTex.setSwizzle(swizzle); // To make debugging easier on the eyes
            shadowFbo.create();
            shadowFbo.setTexture(shadowTex, FrameBufferAttachmentType::Depth);
            shadowShader.create(myEmbeds.at("TLib/Embed/Shaders/light.vert").asString(),
                                myEmbeds.at("TLib/Embed/Shaders/empty.frag").asString());
        }
    }

};

using R3D = Renderer3D;

struct ModelInstance
{
    Transform3D transform;
    Mesh*       modelPtr = nullptr;
};

Vector<Vector3f> lines;

struct PlayerController
{
public:
    Vector2f moveDir;
    Vector3f up = Vector3f::up();

    // Input
    bool jump    = false;
    bool primary = false;

    Camera3D camera;
    Quat     bodyRot;
    bool     freeCam      = false;
    float    sens         = 0.1f;
    float    freeCamSpeed = 12.f;
    float    yaw          = 0.f;
    float    pitch        = 0.f;

    float maxPitch =  89.f;
    float minPitch = -89.f;

    float    moveSpeed =  0.01f;
    float    gravity   = -0.011f;
    float    friction  =  0.92f;
    float    jumpPower =  0.25f;
    Vector3f velocity;

    PxController* playerController = nullptr;
    float         playerHeight     = 0.8f;

    bool isOnGround = false;
    bool lookingAtGeometry = false;

    Vector3f getPos() const
    {
        return Vector3f(playerController->getPosition());
    }

    void update(float delta, bool mouseCaptured)
    {
        if (mouseCaptured)
        {
            yaw   += float(Input::mouseDelta.x) * sens;
            pitch += float(Input::mouseDelta.y) * sens;
            pitch  = std::clamp(pitch, minPitch, maxPitch);
        }

        moveDir.x = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
        moveDir.y = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);

        auto yawRad   = glm::radians(yaw);
        auto pitchRad = glm::radians(pitch);

        Quat qPitch  (pitchRad, Vector3f(1, 0, 0));
        Quat qYaw    (yawRad,   Vector3f(0, 1, 0));
        camera.rot  = (qPitch * qYaw).normalized();
        bodyRot     = qYaw;

        // Camera Look
        auto forward = camera.rot.forward();
        if (freeCam)
        {
            Vector3f finalMovement{};
            if (moveDir.x) finalMovement += forward.cross(up).normalized() * moveDir.x * delta;
            if (moveDir.y) finalMovement += forward * moveDir.y * delta;

            camera.pos += finalMovement * freeCamSpeed;
        }
        else
        {
            const auto& pos = playerController->getPosition();
            camera.pos = Vector3f(pos.x, pos.y + playerHeight/2.f, pos.z);
        }

        if (!freeCam && Input::isKeyJustPressed(SDL_SCANCODE_SPACE))
        { jump = true; }

        if (Input::isMouseJustPressed(Input::MOUSE_LEFT))
        { primary = true; }

    }

    void fixedUpdate(float delta)
    {
        if (freeCam) { }
        else
        {
            const auto& pos = playerController->getPosition();
            PxControllerState state;
            playerController->getState(state);

            Vector3f fw = bodyRot.forward();
            if (moveDir.x) velocity += fw.cross(up).normalized() * moveDir.x * moveSpeed;
            if (moveDir.y) velocity += fw * moveDir.y * moveSpeed;

            auto* scene = playerController->getScene();

            PxRaycastBuffer hit;
            PxQueryFilterData d; d.flags = PxQueryFlag::eSTATIC;
            lookingAtGeometry = false; //scene->raycast(PxVec3(camera.pos.x, camera.pos.y, camera.pos.z), PxVec3(forward.x, forward.y, forward.z), 10000.f, hit, PxHitFlag::eDEFAULT, d);

            if (primary)
            {
                primary = false;
                if (lookingAtGeometry)
                {
                    lines.emplace_back(camera.pos);
                    lines.emplace_back(hit.block.position.x, hit.block.position.y, hit.block.position.z);
                }
            }

            if (jump)
            {
                jump = false;
                if (isOnGround)
                { velocity.y += jumpPower; isOnGround = false; }
            }
            else if (!isOnGround)
            { velocity.y += gravity; }
            else
            { velocity.y = gravity; }

            velocity.x *= friction;
            velocity.z *= friction;

            PxControllerFilters filters;
            filters.mFilterFlags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;
            PxControllerCollisionFlags flags = playerController->move(PxVec3(velocity.x, velocity.y, velocity.z), 0.002f, delta, filters);
            isOnGround = flags & PxControllerCollisionFlag::eCOLLISION_DOWN;
        }
    }

    void resetCamera()
    {
        camera     = Camera3D();
        pitch      = 0;
        yaw        = 0;
    }
};

struct MeshInstance3D
{
    Asset<Mesh> mesh;
    Path        path;

    MeshInstance3D(const Path& path) : path{ path } { }
};

struct RigidBody3D
{

};

struct Scene
{
    /* Components:
        Transform3D
        MeshInstance3D

        Physics:
            RigidBody3D
    */
    using Entity = flecs::entity;
    using ECS    = flecs::world;
    using System = flecs::system;

    ECS ecs;

    System sysMeshInstance3DRender;

    #pragma region Constructors/Destructors

    void onSetMeshInstance3D(Entity e, MeshInstance3D& meshInstance)
    {
        tlog::info("Creating mesh instance: {}", meshInstance.path.string());
        meshInstance.mesh = new Mesh();
        meshInstance.mesh->loadFromFile(meshInstance.path);
    }

    void onSetRigidBody3D(Entity e, RigidBody3D& body)
    {

    }

    #pragma endregion

    #pragma region Systems

    void sysMeshRenderFunc(Entity e, MeshInstance3D& mesh, const Transform3D& tf)
    {
        R3D::drawModel(mesh.mesh.get(), tf);
    }

    #pragma endregion

    void init()
    {
        ecs.observer<MeshInstance3D>("OnSetMeshInstance3D").event(flecs::OnSet).each(
        [&](Entity e, MeshInstance3D& meshInstance) { onSetMeshInstance3D(e, meshInstance); });

        sysMeshInstance3DRender = ecs.system<MeshInstance3D, const Transform3D>("MeshInstance3D Render").each(
        [&](Entity e, MeshInstance3D& mesh, const Transform3D& tf) { sysMeshRenderFunc(e, mesh, tf); });

        auto e = ecs.entity();
        ASSERT(e.is_alive());

        e.emplace<Transform3D>(Vector3f::up() * 5.f);
        e.emplace<MeshInstance3D>("assets/primitives/cube.obj");
    }

    void update(float delta)
    {

    }

    void render(float delta)
    {
        sysMeshInstance3DRender.run(delta);
    }
};

Window     window;
MyGui      imgui;
FPSLimit   fpslimit;
Timer      deltaTimer;

String vertShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString();
String fragShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString();

bool debugDrawPhysics = false;
Vector<ModelInstance> models;
PlayerController controller;

const char* presetModelPaths[7] =
{
    "assets/primitives/cube.obj",
    "assets/sponza/sponza.obj",
    "assets/backpack/backpack.obj",
    "assets/primitives/cylinder.obj",
    "assets/primitives/monkey.obj",
    "assets/primitives/sphere.obj",
    "assets/primitives/torus.obj"
};
int guiSelectedModel = 0;
String guiModel;

ColorRGBAf lightColor ("#FFE082");
ColorRGBAf sunColor   ("#17163A");
ColorRGBAf spotColor  ("#FFC182");
Vector3f   sunDir     = Vector3f::down();
float      lightPower = 1.f;
float      sunPower   = 1.f;
float      spotPower  = 200.f;
float      spotAngle  = 10.f;
float      spotOuterAngle = 30.f;

float totalTime = 0.f;

PxPhysics*           physics          = nullptr;
PxScene*             physicsScene     = nullptr;
PxControllerManager* controllerMan    = nullptr;

Scene scene;

void init()
{
    initPhysics();

    PxTolerancesScale scale{};
    physics = createPhysics(scale);

    physicsScene = createPhysicsScene(physics);

    PxShapeFlags    shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
    PxMaterial*     mat = physics->createMaterial(1, 1, 1);
    PxCookingParams params(physics->getTolerancesScale());

    // Load level mesh
    MeshData levelMesh;
    levelMesh.loadFromFile("assets/scuffed_construct.glb");

    // Level collision
    for (auto& submesh : levelMesh.subMeshes)
    {
        PxTriangleMeshDesc meshDesc;

        Vector<Vector3f> vertices;
        for (auto& vert : submesh.vertices)
        {
            vertices.emplace_back(vert.position);
        }

        meshDesc.points.count           = vertices.size();;
        meshDesc.points.stride          = sizeof(Vector3f);
        meshDesc.points.data            = vertices.data();
        meshDesc.triangles.count        = submesh.indices.size();
        meshDesc.triangles.stride       = 3*sizeof(uint32_t);
        meshDesc.triangles.data         = submesh.indices.data();

        PxDefaultMemoryOutputStream writeBuffer;
        bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, NULL);
        ASSERT(status);

        PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

        PxTriangleMesh* triMesh = physics->createTriangleMesh(readBuffer);
        
        PxMeshScale triScale(physx::PxVec3(1.0f, 1.0f, 1.0f), physx::PxQuat(physx::PxIdentity));
        PxRigidStatic* rigidStatic = physics->createRigidStatic(PxTransform({0, 0, 0}));
        {
            PxShape* shape = physics->createShape(PxTriangleMeshGeometry(triMesh, triScale), *mat, true, shapeFlags);
            shape->setContactOffset(0.002f);
            shape->setRestOffset(0.002f);
            rigidStatic->attachShape(*shape);
            shape->release(); // this way shape gets automatically released with actor
        }

        physicsScene->addActor(*rigidStatic);
    }

    // Upload to GPU
    auto& m = models.emplace_back();
    m.modelPtr = new Mesh();
    m.modelPtr->loadFromMemory(levelMesh);

    // Player geometry
    PxMaterial* playerMat = physics->createMaterial(0, 0, 0);
    controllerMan = PxCreateControllerManager(*physicsScene);
    PxCapsuleControllerDesc cdesc;
    cdesc.material      = playerMat;
    cdesc.radius        = 0.5f;
    cdesc.height        = controller.playerHeight;
    cdesc.contactOffset = 0.01f;
    controller.playerController = controllerMan->createController(cdesc);

    scene.init();
}

void fixedUpdate(float delta)
{
    controller.fixedUpdate(delta);
    physicsScene->simulate(delta);
    physicsScene->fetchResults(true);
}

void update(float delta)
{
    // Fixed update
    const  float fixedTimeStep  = 1.f/60.f;
    static float time           = 0;
    static float lastUpdateTime = 0;
    static float timeBuffer     = 0;
    time += delta;
    timeBuffer += time - lastUpdateTime;
    lastUpdateTime = time;
    while (timeBuffer >= fixedTimeStep)
    {
        fixedUpdate(fixedTimeStep);
        timeBuffer -= fixedTimeStep;
    }

    scene.update(delta);

    #pragma region UI
    beginDiagWidgetExt();

    if (ImGui::CollapsingHeader("Character & Camera"))
    {
        if (ImGui::Checkbox("Physics Debug", &debugDrawPhysics))
        {
            if (debugDrawPhysics)
            {
                physicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f); // Required
                physicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
            }
            else
            {
                physicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 0.f);
                physicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 0.f);
            }
        }

        ImGui::Checkbox("Freecam", &controller.freeCam);

        ImGui::SliderFloat("Move Speed", &controller.moveSpeed, 0.f, 100.f);
        ImGui::SliderFloat("Gravity", &controller.gravity, -2.f, 2.f);
        ImGui::SliderFloat("Friction", &controller.friction, 0.f, 1.f);
        ImGui::SliderFloat("Jump Power", &controller.jumpPower, 1.f, 100.f);
        ImGui::Checkbox   ("On Ground", &controller.isOnGround);

        ImGui::Text(fmt::format("Velocity: {}", controller.velocity.toString()).c_str());
        ImGui::Text(fmt::format("Looking At Geometry: {}", controller.lookingAtGeometry).c_str());
        //ImGui::Text(fmt::format("Dir:      {}", Vector3f(controller.forward).toString()).c_str());
        //ImGui::Text(fmt::format("Dir No Y: {}", Vector3f(controller.forwardNoY).toString()).c_str());

        Vector3f playerPos(controller.playerController->getPosition());
        static Vector3f guiTeleport;
        ImGui::InputFloat3("##teleportinput", &guiTeleport.x);
        if (ImGui::Button("Teleport"))
        { controller.playerController->setPosition({ guiTeleport.x, guiTeleport.y, guiTeleport.z }); }
        ImGui::SameLine();
        if (ImGui::Button("Save Pos"))
        { guiTeleport = playerPos; }
        ImGui::Text(fmt::format("Current Pos: {}", playerPos.toString()).c_str());

        static Vector3f guiGravity;
        ImGui::InputFloat3("##gravinput", &guiGravity.x);
        if (ImGui::Button("Set Gravity"))
        { physicsScene->setGravity({ guiGravity.x, guiGravity.y, guiGravity.z }); }

        ImGui::SeparatorText("Camera");
        if (ImGui::Button("Reset Camera"))
        { controller.resetCamera(); }
        ImGui::SliderFloat("FOV", &controller.camera.fov, 70.f, 170.f);

        ImGui::Text("Press ALT to toggle cursor.");
        ImGui::SliderFloat("Sensitivity", &controller.sens, 0.01f, 2.f);
        ImGui::SliderFloat("Camera Speed", &controller.freeCamSpeed, 0.01f, 400.f);
        ImGui::Text(fmt::format("Pitch : {}", controller.pitch).c_str());
        ImGui::Text(fmt::format("Yaw   : {}", controller.yaw).c_str());
    }
    // Model loading input
    //ImGui::SeparatorText("Model");
    //if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
    //{ /*model.loadFromFile(presetModelPaths[guiSelectedModel]);*/ }
    //if (ImGui::Button("Load Custom Model"))
    //{
    //    Path p = openSingleFileDialog();
    //    if (!p.empty()) { model.loadFromFile(p); }
    //}

    if (ImGui::CollapsingHeader("Graphics"))
    {
        ImGui::ColorEdit3("Player Light",       &lightColor.r);
        ImGui::DragFloat ("Player Light Power", &lightPower);
        ImGui::ColorEdit3("Sun Light",          &sunColor.r);
        ImGui::DragFloat3("Sun Dir",            &sunDir.x, 0.025f, -1.f, 1.f);
        ImGui::DragFloat ("Sun Power",          &sunPower);
        ImGui::ColorEdit3("Spot Light",         &spotColor.r);
        ImGui::DragFloat ("Spot Power",         &spotPower);
        ImGui::DragFloat ("Spot Angle",         &spotAngle);
        ImGui::DragFloat ("Spot Outer Angle",   &spotOuterAngle);
        

        static float ambientStrength = R3D::shader3d.getFloat("ambientStrength");
        if (ImGui::SliderFloat("Ambient Lighting", &ambientStrength, 0.f, 1.f))
        { R3D::shader3d.setFloat("ambientStrength", ambientStrength); }

        ImGui::Checkbox("MSAA", &R3D::useMSAA);

        if (ImGui::Button("Compile Shaders"))
        { R3D::setPBRShader(vertShaderStr, fragShaderStr); }
        if (ImGui::CollapsingHeader("Vert Shader"))
        { ImGui::InputTextMultiline("#VertShaderEdit", &vertShaderStr, { ImGui::GetContentRegionAvail().x, 600 }); }
        if (ImGui::CollapsingHeader("Frag Shader"))
        { ImGui::InputTextMultiline("#FragShaderEdit", &fragShaderStr, { ImGui::GetContentRegionAvail().x, 600 }); }
        if (ImGui::CollapsingHeader("Shadow Map"))
        { ImGui::Image(R3D::shadowTex, Vector2f(128.f, 128.f) * 4.f); }
    }
    ImGui::End();
    #pragma endregion

    // Camera
    {
        if (Input::isKeyJustPressed(SDL_SCANCODE_LALT))
        { window.toggleFpsMode(); }

        controller.update(delta, window.getFpsMode());

        R3D::setCamera(controller.camera);
    }
}

void draw(float delta)
{
    Vector3f lightPos  = controller.getPos();
    R3D::addPointLight(lightPos, ColorRGBf(lightColor), lightPower);
    R3D::addDirectionalLight(sunDir, ColorRGBf(sunColor), sunPower);
    //R3D::addSpotLight(lightPos, Vector3f(controller.forward), spotAngle, spotOuterAngle, ColorRGBf(spotColor), spotPower);

    Vector3f soffset = Vector3f::up() * 4.f;
    R3D::drawLines(std::initializer_list{ soffset, soffset + sunDir*2.f }, sunColor, GLDrawMode::Lines);

    for (auto& modelInst : models)
    {
        ASSERT(modelInst.modelPtr);
        R3D::drawModel(*modelInst.modelPtr, modelInst.transform);
    }

    scene.render(delta);

    R3D::drawLines(std::initializer_list{ Vector3f::up()      * 10, Vector3f() }, ColorRGBAf::green(), GLDrawMode::Lines);
    R3D::drawLines(std::initializer_list{ Vector3f::right()   * 10, Vector3f() }, ColorRGBAf::red(),   GLDrawMode::Lines);
    R3D::drawLines(std::initializer_list{ Vector3f::forward() * 10, Vector3f() }, ColorRGBAf::blue(),  GLDrawMode::Lines);
    R3D::drawLines(lines, ColorRGBAf::green(), GLDrawMode::Lines);

    if (debugDrawPhysics)
    {
        const PxRenderBuffer& rb = physicsScene->getRenderBuffer();
        for (PxU32 i=0; i < rb.getNbPoints(); i++)
        {
            const PxDebugPoint& point = rb.getPoints()[i];
            // render the point
        }

        static Vector<Vector3f> lines;
        lines.clear();
        lines.reserve(rb.getNbLines());
        for (PxU32 i=0; i < rb.getNbLines(); i++)
        {
            const PxDebugLine& line = rb.getLines()[i];
            // render the line
            lines.emplace_back(line.pos0.x, line.pos0.y, line.pos0.z);
            lines.emplace_back(line.pos1.x, line.pos1.y, line.pos1.z);
        }
        if (!lines.empty())
            R3D::drawLines(lines, ColorRGBAf::white(), GLDrawMode::Lines);
    }
    
}

int main()
{
    WindowCreateParams p;
    p.size  = { 1600, 900 };
    p.title = "Construct";
    window.create(p);
    Renderer::create();

    Renderer2D::create();
    
    R3D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    init();

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = Renderer2D::getView();
                view.size = Vector2f(e.window.data1, e.window.data2);
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        imgui.newFrame();
        ImGui::BeginDisabled(window.getFpsMode());

        Vector2f mwpos = Renderer2D::getMouseWorldPos();

        glClear(GL_DEPTH_BUFFER_BIT);
        Renderer::clearColor();
        totalTime += delta;
        update(delta);
        draw(delta);
        R3D::render();
        Renderer2D::render();
        drawDiagWidget(&fpslimit);
        ImGui::EndDisabled();
        imgui.render();
        window.swap();
        fpslimit.wait();
    }

    return 0;
}