
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/ECS/ECS.hpp>
#include <TLib/Physics3D.hpp>
#include <TLib/Media/Transform3D.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Media/Resource/MeshData.hpp>
#include <TLib/Media/Renderer3D.hpp>

struct BodyTransform3D
{
    Vector3f pos;
    Quat     rot;
};

struct Physics3DWorld : NonAssignable
{
public:
    // PxPhysics is a singleton, don't need to free it.
    Physics3D::PxPhysics* phys  = nullptr;
    Physics3D::PxScene*   scene = nullptr;

    Physics3D::PhysXMaterial toPhysXMat(const Physics3D::Material& mat)
    {
        Physics3D::PhysXMaterial ret;
        ret.reset(phys->createMaterial(mat.staticFriction, mat.dynamicFriction, mat.restitution));
        return ret;
    }

public:
    ~Physics3DWorld()
    { reset(); }

    void init()
    {
        using namespace Physics3D;
        reset();

        if (!physicsInited()) { initPhysics(); }

        PxTolerancesScale scale{};
        phys  = createPhysics(scale);
        scene = createPhysicsScene(phys);
    }

    void reset()
    {
        if (scene)
        {
            scene->release();
            scene = nullptr;
        }
    }

    bool inited() const
    { return scene; }

    void setGravity(const Vector3f& gravity)
    { scene->setGravity(Physics3D::PxVec3(gravity.x, gravity.y, gravity.z)); }

    Vector3f getGravity() const
    { return Vector3f(scene->getGravity()); }

    // TODO: https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/BestPractices.html#performance-issues
    void simulate(float delta)
    {
        scene->simulate    (delta);
        scene->fetchResults(true);
    }

    Physics3D::PxShape* createShape(
        const Physics3D::PxGeometry& geometry,
        const Physics3D::Material&   material = Physics3D::Material())
    {
        using namespace Physics3D;
        PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
        return phys->createShape(geometry, *toPhysXMat(material), false, shapeFlags);
    }

    static void freeRigidBody(Physics3D::PxRigidDynamic* body)
    { if (body) body->release(); }

    static void freeShape(Physics3D::PxShape* shape)
    { shape->release(); }

    void debugDraw()
    {
        using namespace physx;

        scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f); // Required
        scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

        const PxRenderBuffer& rb = scene->getRenderBuffer();
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
            Renderer3D::drawLines(lines, ColorRGBAf::white(), GLDrawMode::Lines);
    }
};

struct Physics3DBody
{
    Physics3D::Body body    { nullptr };
    Physics3DWorld* world   = nullptr;
    bool           _enabled = false;

    void init(Physics3DWorld& world)
    {
        this->world = &world;
    }

    bool inited() const { return world; }

    void reset()
    {
        if (world && body) world->scene->removeActor(*body);
        body.reset();
        _enabled = false;
    }

    void makeRigidBody(const Vector3f& pos = { 0, 0, 0 })
    {
        body.reset(world->phys->createRigidDynamic({ pos.x, pos.y, pos.z }));
        enable();
    }

    void makeStaticBody(const Vector3f& pos = { 0, 0, 0 })
    {
        body.reset(world->phys->createRigidStatic({ pos.x, pos.y, pos.z }));
        enable();
    }

    #pragma region Colliders

    void addBoxCollider(const Vector3f& halfExtents = { 1.f, 1.f, 1.f }, const Physics3D::Material& material = Physics3D::Material())
    {
        Physics3D::PxBoxGeometry geom(halfExtents.x, halfExtents.y, halfExtents.z);
        auto shape = world->phys->createShape(geom, *world->toPhysXMat(material), true);
        ASSERT(shape);
        body->attachShape(*shape);
        shape->release();
    }

    // Adds all submeshes as separate colliders
    void addTriMeshCollider(const MeshData& mesh, const Physics3D::Material& material = Physics3D::Material())
    {
        for (auto& subMesh : mesh.subMeshes)
        { addTriMeshCollider(subMesh, material); }
    }

    void addTriMeshCollider(const MeshData::SubMesh& submesh, const Physics3D::Material& material = Physics3D::Material())
    {
        // https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/Geometry.html
        using namespace Physics3D;

        // Create Geometry
        PxTriangleMesh* triMesh = nullptr;
        {
            static Vector<Vector3f> vertices;
            ASSERT(vertices.empty());
            vertices.reserve(submesh.vertices.size());
            for (auto& vert : submesh.vertices)
            { vertices.emplace_back(vert.position); }

            PxTriangleMeshDesc meshDesc;
            meshDesc.points.count     = vertices.size();;
            meshDesc.points.stride    = sizeof(Vector3f);
            meshDesc.points.data      = vertices.data();
            meshDesc.triangles.count  = submesh.indices.size()/3;
            meshDesc.triangles.stride = 3*sizeof(uint32_t);
            meshDesc.triangles.data   = submesh.indices.data();
            ASSERT(meshDesc.isValid());

            PxCookingParams params(world->phys->getTolerancesScale());
            //ASSERT(PxValidateTriangleMesh(params, meshDesc));

            triMesh = PxCreateTriangleMesh(params, meshDesc);
            vertices.clear();
        }
        PxMeshScale triScale(physx::PxVec3(1.0f, 1.0f, 1.0f), physx::PxQuat(physx::PxIdentity));
        PxTriangleMeshGeometry geom(triMesh, triScale);

        // Attach Geometry
        auto shape = world->phys->createShape(geom, *world->toPhysXMat(material), true);
        body->attachShape(*shape);
        shape->release();
    }

    #pragma endregion

    bool enabled() const { return _enabled; }

    void enable()
    {
        ASSERT(inited());
        if (enabled()) { return; } // Already enabled
        world->scene->addActor(*body);
        _enabled = true;
    }

    void disable()
    {
        if (!enabled()) { return; } // Already disabled
        world->scene->removeActor(*body);
        _enabled = false;
    }

    void setPosition(const Vector3f& pos)
    {
        Physics3D::PxTransform tf = body->getGlobalPose();
        tf.p ={ pos.x, pos.y, pos.z };
        body->setGlobalPose(tf);
    }

    Vector3f getPosition() const
    {
        Physics3D::PxTransform tf = body->getGlobalPose();
        return Vector3f(tf.p.x, tf.p.y, tf.p.z);
    }

    BodyTransform3D getTransform() const
    {
        BodyTransform3D ret;
        Physics3D::PxTransform tf = body->getGlobalPose();
        ret.pos = Vector3f(tf.p.x, tf.p.y, tf.p.z);
        ret.rot = Quat(tf.q.w, tf.q.x, tf.q.y, tf.q.z);
        return ret;
    }
};

static void Physics3DBody_onFixedUpdate(Entity e, const Physics3DBody& body, Transform3D& tf)
{
    auto bodyTf = body.getTransform();
    tf.pos      = bodyTf.pos;
    tf.rot      = bodyTf.rot;
}
