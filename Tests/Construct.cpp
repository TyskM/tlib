
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
    glm::vec3 pos      ={ 0.0f, 0.0f, 0.0f };
    glm::vec3 target   ={ 0.0f, 0.0f, 0.0f };
    glm::vec3 up       ={ 0.0f, 1.0f, 0.0f };
    ViewMode  viewmode = ViewMode::Perspective;

    inline void setPos(const Vector3f& posv) { this->pos ={ posv.x, posv.y, posv.z }; }
    [[nodiscard]] inline Vector3f getPos() const { return Vector3f(pos); }

    inline void setTarget(const Vector3f& targetv) { target ={ targetv.x, targetv.y, targetv.z }; }
    [[nodiscard]] inline Vector3f getTarget() const { return Vector3f(target); }

    inline void setUp(const Vector3f& upv) { up ={ upv.x, upv.y, upv.z }; }
    [[nodiscard]] inline Vector3f getUp() const { return Vector3f(up); }

    [[nodiscard]]
    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(pos, target, up);
    }

    [[nodiscard]]
    glm::mat4 getPerspectiveMatrix() const
    {
        Vector2f size = Vector2f(Renderer::getFramebufferSize());
        switch (viewmode)
        {
            case ViewMode::Perspective:
                return glm::perspective(glm::radians(fov), size.x / size.y, znear, zfar); break;
            case ViewMode::Orthographic:
                return glm::ortho(0.f, size.x, 0.f, size.y, znear, zfar); break;
            default: return glm::mat4(); break;
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

struct Quat : glm::quat
{
    using glm::quat::quat;
};

struct Transform3D
{
    Vector3f  pos;
    Vector3f  scale = { 1, 1, 1 };
    Quat      rot     { glm::vec3(0.f, 0.f, 0.f) };

    glm::mat4 getMatrix() const
    {
        // Scale, rotate, translate
        glm::mat4 mat(1.f);
        mat  = glm::scale(mat, { scale.x, scale.y, scale.z });
        mat *= glm::toMat4(rot);
        mat  = glm::translate(mat, { pos.x, pos.y, pos.z });
        return mat;
    }

    void rotate(float angle, const Vector3f& axis)
    {
        rot *= glm::angleAxis(angle, glm::vec3{ axis.x, axis.y, axis.z });
    }
};

class Renderer3D
{
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

public:
    static inline Vector<DrawCmd> cmds;

    static inline Shader  shader3d;

    static inline Texture emptyTex;
    static inline GLubyte emptyTexData[1][1][4] ={ {{255,255,255,255}} };

    static inline Texture defaultTex;
    static constexpr int  defaultTexDataSize = 32;
    static inline GLubyte defaultTexColor[4] ={ 255, 255, 255, 255 };
    static inline GLubyte defaultTexData[defaultTexDataSize][defaultTexDataSize][4];

    static inline bool useMSAA      = true;  // Read / Write. Creates ugly artifacts (sometimes), find post processing solution.
    static inline bool useDithering = false; // Read / Write. Is a no op on most implementations.

private:

    static inline FrameBuffer shadowFbo;
    static inline Texture     shadowTex;
    static inline uint32_t    shadowSize = 1024;
    static inline Shader      shadowShader;

    static void drawLightDepthMap()
    {
        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView =
            glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    }

    static void drawMeshImmediate(GPUVertexData& mesh, Texture& tex, const Transform3D& transform)
    {
        tex.bind();
        emptyTex.bind(1);
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
    static void render()
    {
        if (cmds.empty()) { return; }

        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glFrontFace(GL_CCW));
        GL_CHECK(glEnable(GL_DITHER));

        if (useDithering) { GL_CHECK(glEnable(GL_DITHER)); }
        else { GL_CHECK(glDisable(GL_DITHER)); }

        if (useMSAA) { GL_CHECK(glEnable(GL_MULTISAMPLE)); }
        else { GL_CHECK(glDisable(GL_MULTISAMPLE)); }

        shader3d.setInt ("material.diffuse",   0);
        shader3d.setInt ("material.roughness", 1);
        shader3d.setInt ("material.metallic",  2);

        //Renderer::setViewport(Recti(Vector2i(0), Vector2i(shadowSize, shadowSize)));
        //shadowFbo.bind();
        //glClear(GL_DEPTH_BUFFER_BIT);
        //// Shadows here
        //shadowFbo.unbind();
        //Renderer2D::setView(Renderer2D::getView());

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        bool     stateChanged = true;
        uint32_t primOffset = 0;

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

                //auto start = primitiveIndices.begin() + primOffset;
                //auto end   = start + cmd.indSize - 1;
                //flushPrimitives(*cmd.shader, cmd.drawMode, Span<uint32_t>(start, end));
                primOffset = cmd.indIndex + cmd.indSize;
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
    }

    static void create()
    {
        primitiveMesh.setLayout({ Layout::Vec3f(), Layout::Vec4f() });
        defaultPrimitiveShader.create(
            myEmbeds.at("TLib/Embed/Shaders/3d_primitive.vert").asString(),
            myEmbeds.at("TLib/Embed/Shaders/3d_primitive.frag").asString());
        defaultPrimitiveShader.setMat4f("model", glm::mat4(1));
        primitiveVerts   .reserve(1024 * 4);
        primitiveIndices .reserve(1024 * 4);

        defaultTex.create();

        shader3d.create(myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString(),
                        myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString());

        emptyTex.create();
        emptyTex.setData(emptyTexData, 1, 1);

        // Setup shadows
        shadowTex.create();
        shadowTex.setData(NULL, shadowSize, shadowSize, TexPixelFormats::DEPTH_COMPONENT, TexInternalFormats::DEPTH_COMPONENT);
        shadowTex.setFilter(TextureFiltering::Nearest);
        shadowTex.setUVMode(UVMode::Repeat);
        shadowFbo.create();
        shadowFbo.setTexture(shadowTex, FrameBufferAttachmentType::Depth);
        shadowShader.create(myEmbeds.at("TLib/Embed/Shaders/light.vert").asString(),
            myEmbeds.at("TLib/Embed/Shaders/empty.frag").asString());

        // Make the default texture look more interesting
        RNG rng;
        for (size_t x = 0; x < defaultTexDataSize; x++)
        {
            for (size_t y = 0; y < defaultTexDataSize; y++)
            {
                const int diff = 20;
                GLubyte color[3] ={ 70, 130, 180 };
                color[0] = std::clamp(rng.randRangeInt(color[0] - diff, color[0] + diff), 0, 255);
                color[1] = std::clamp(rng.randRangeInt(color[1] - diff, color[1] + diff), 0, 255);
                color[2] = std::clamp(rng.randRangeInt(color[2] - diff, color[2] + diff), 0, 255);

                defaultTexData[x][y][0] = color[0];
                defaultTexData[x][y][1] = color[1];
                defaultTexData[x][y][2] = color[2];
                defaultTexData[x][y][3] = 255;
            }
        }
        defaultTex.setData(&defaultTexData, defaultTexDataSize, defaultTexDataSize);
        defaultTex.setUVMode(UVMode::Repeat);
        defaultTex.setFilter(TextureFiltering::Linear);
    }

    static bool created()
    {
        return shader3d.created();
    }

    static void setCamera(const Camera3D& camera)
    {
        // TODO: use one of those global uniform things
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

    static void addLight(const Vector3f& pos, const ColorRGBf& color)
    {
        // TODO: global uniform things here too.
        shader3d.setVec3f("lightPos", pos.x, pos.y, pos.z);
        shader3d.setVec3f("lightColor", color.r, color.g, color.b);
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
    Vector2f  moveDir;
    glm::vec3 forward{};
    glm::vec3 forwardNoY{};
    Vector3f up {0.f, 1.f, 0.f};

    // Input
    bool jump    = false;
    bool primary = false;

    Camera3D camera;
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

    void update(float delta, bool mouseCaptured)
    {
        if (mouseCaptured)
        {
            yaw   += float(Input::mouseDelta.x) * sens;
            pitch -= float(Input::mouseDelta.y) * sens;
            pitch  = std::clamp(pitch, minPitch, maxPitch);
        }

        moveDir.x = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
        moveDir.y = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);

        forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward.y = sin(glm::radians(pitch));
        forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward   = glm::normalize(forward);

        forwardNoY.x = cos(glm::radians(yaw));
        forwardNoY.z = sin(glm::radians(yaw));

        // Camera Look
        if (freeCam)
        {
            glm::vec3 finalMovement{};
            if (moveDir.x) finalMovement += glm::normalize(glm::cross(forward, camera.up)) * moveDir.x * delta;
            if (moveDir.y) finalMovement += forward * moveDir.y * delta;

            camera.pos += finalMovement * freeCamSpeed;
        }
        else
        {
            const auto& pos = playerController->getPosition();
            camera.pos = { pos.x, pos.y + playerHeight/2.f, pos.z };
        }
        camera.target = camera.pos + forward;

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

            Vector3f fw(forwardNoY);
            if (moveDir.x) velocity += fw.cross(up).normalized() * moveDir.x * moveSpeed;
            if (moveDir.y) velocity += fw * moveDir.y * moveSpeed;

            auto* scene = playerController->getScene();

            PxRaycastBuffer hit;
            PxQueryFilterData d; d.flags = PxQueryFlag::eSTATIC;
            lookingAtGeometry = scene->raycast(PxVec3(camera.pos.x, camera.pos.y, camera.pos.z), PxVec3(forward.x, forward.y, forward.z), 10000.f, hit, PxHitFlag::eDEFAULT, d);

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

        e.emplace<Transform3D>();
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
float guiLightColor[3] ={ 0.5f, 0.5f, 0.5f };
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

    ImGui::SliderFloat("Move Speed", &controller.moveSpeed,         0.f,  100.f);
    ImGui::SliderFloat("Gravity",    &controller.gravity,          -2.f,  2.f);
    ImGui::SliderFloat("Friction",   &controller.friction,          0.f,  1.f);
    ImGui::SliderFloat("Jump Power", &controller.jumpPower,         1.f,  100.f);
    ImGui::Checkbox   ("On Ground",  &controller.isOnGround);

    ImGui::Text(fmt::format("Velocity: {}", controller.velocity.toString()).c_str());
    ImGui::Text(fmt::format("Looking At Geometry: {}", controller.lookingAtGeometry).c_str());
    ImGui::Text(fmt::format("Dir:      {}", Vector3f(controller.forward).toString()).c_str());
    ImGui::Text(fmt::format("Dir No Y: {}", Vector3f(controller.forwardNoY).toString()).c_str());

    Vector3f playerPos(controller.playerController->getPosition());
    static Vector3f guiTeleport;
    ImGui::InputFloat3("##teleportinput", &guiTeleport.x);
    if (ImGui::Button("Teleport"))
    { controller.playerController->setPosition({guiTeleport.x, guiTeleport.y, guiTeleport.z}); }
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
    ImGui::SliderFloat("Sensitivity",  &controller.sens, 0.01f, 2.f);
    ImGui::SliderFloat("Camera Speed", &controller.freeCamSpeed, 0.01f, 400.f);
    ImGui::Text(fmt::format("Pitch : {}", controller.pitch).c_str());
    ImGui::Text(fmt::format("Yaw   : {}", controller.yaw).c_str());

    // Model loading input
    ImGui::SeparatorText("Model");
    if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
    { /*model.loadFromFile(presetModelPaths[guiSelectedModel]);*/ }
    //if (ImGui::Button("Load Custom Model"))
    //{
    //    Path p = openSingleFileDialog();
    //    if (!p.empty()) { model.loadFromFile(p); }
    //}

    ImGui::SeparatorText("Shaders");

    ImGui::ColorEdit3("Lighting", guiLightColor);

    ImGui::Checkbox("MSAA", &R3D::useMSAA);
    ImGui::Checkbox("Dithering", &R3D::useDithering);

    static float ambientStrength = R3D::shader3d.getFloat("ambientStrength");
    if (ImGui::SliderFloat("Ambient Lighting", &ambientStrength, 0.f, 1.f))
    { R3D::shader3d.setFloat("ambientStrength", ambientStrength); }

    static float specularStrength = R3D::shader3d.getFloat("specularStrength");
    if (ImGui::SliderFloat("Specular Strength", &specularStrength, 0.f, 1.f))
    { R3D::shader3d.setFloat("specularStrength", specularStrength); }

    static glm::vec3 sunDir = R3D::shader3d.getVec3f("sunDir");
    if (ImGui::DragFloat3("Sun Dir", &sunDir.x, 0.025f, -1.f, 1.f))
    { R3D::shader3d.setVec3f("sunDir", sunDir); }

    if (ImGui::Button("Compile Shaders"))
    { R3D::shader3d.create(vertShaderStr, fragShaderStr); }
    if (ImGui::CollapsingHeader("Vert Shader"))
    { ImGui::InputTextMultiline("#VertShaderEdit", &vertShaderStr, { ImGui::GetContentRegionAvail().x, 600 }); }
    if (ImGui::CollapsingHeader("Frag Shader"))
    { ImGui::InputTextMultiline("#FragShaderEdit", &fragShaderStr, { ImGui::GetContentRegionAvail().x, 600 }); }

    ImGui::SeparatorText("Diag");
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
    const float radius = 20.0f;
    float    lightX    = sin(totalTime) * radius;
    float    lightZ    = cos(totalTime) * radius;
    Vector3f lightPos  ={ lightX, 0.f, lightZ };
    R3D::addLight(lightPos, { guiLightColor[0], guiLightColor[1], guiLightColor[2] });

    for (auto& modelInst : models)
    {
        ASSERT(modelInst.modelPtr);
        R3D::drawModel(*modelInst.modelPtr, modelInst.transform);
    }

    scene.render(delta);

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