
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/ECS/ECS.hpp>
#include <TLib/Physics3D.hpp>
#include <TLib/Media/Transform3D.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Media/Resource/MeshData.hpp>

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

public:

    static inline Physics3D::Material defaultMaterial;

    ~Physics3DWorld()
    { reset(); }

    void init()
    {
        using namespace Physics3D;
        reset();

        if (!physicsInited()) { initPhysics(); }

        PxTolerancesScale scale{};
        phys            = createPhysics(scale);
        scene           = createPhysicsScene(phys);
        defaultMaterial = createMaterial(0.f, 0.f, 0.f);
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

    void simulate(float delta)
    {
        scene->simulate    (delta);
        scene->fetchResults(true);
    }

    Physics3D::PxShape* createShape(
        const Physics3D::PxGeometry& geometry,
        const Physics3D::Material&   material = defaultMaterial)
    {
        using namespace Physics3D;
        PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
        return phys->createShape(geometry, *material, false, shapeFlags);
    }

    Physics3D::Material createMaterial(float staticFriction, float dynamicFriction, float restitution)
    {
        Physics3D::Material ret;
        ret.reset(phys->createMaterial(staticFriction, dynamicFriction, restitution));
        return ret;
    }

    static void freeRigidBody(Physics3D::PxRigidDynamic* body)
    { if(body) body->release(); }

    static void freeShape(Physics3D::PxShape* shape)
    { shape->release(); }
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

    void addBoxCollider(const Vector3f& halfExtents = { 1.f, 1.f, 1.f })
    {
        Physics3D::PxBoxGeometry geom(halfExtents.x, halfExtents.y, halfExtents.z);
        auto shape = world->phys->createShape(geom, *world->defaultMaterial, true);
        ASSERT(shape);
        body->attachShape(*shape);
        shape->release();
    }

    // Adds all submeshes as separate colliders
    void addTriMeshCollider(const MeshData& mesh)
    {
        for (auto& subMesh : mesh.subMeshes)
        { addTriMeshCollider(subMesh); }
    }

    void addTriMeshCollider(const MeshData::SubMesh& submesh)
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
        auto shape = world->phys->createShape(geom, *world->defaultMaterial, true);
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
        tf.p = { pos.x, pos.y, pos.z };
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

//struct BaseRigidOrStatic3D // Non Copyable
//{
//
//
//
//
//};
//
//struct RigidBody3D : public BaseRigidOrStatic3D
//{
//private:
//    friend void RigidBody3D_onFixedUpdate(Entity e, const RigidBody3D& body, Transform3D& tf);
//
//    using Self = Physics3D::RigidBody::element_type*;
//    Self self() const { return static_cast<Self>(body.get()); }
//};
//
//
//
//struct StaticBody3D : public BaseRigidOrStatic3D
//{
//    StaticBody3D(Physics3DWorld& world, const Vector3f& pos = { 0, 0, 0 }) : BaseRigidOrStatic3D(world)
//    {
//        body.reset(world.createStaticBody(pos));
//        enable();
//    }
//
//private:
//    using Self = Physics3D::StaticBody::element_type*;
//    Self self() const { return static_cast<Self>(body.get()); }
//
//    friend void StaticBody3D_onFixedUpdate(Entity e, const StaticBody3D& body, Transform3D& tf);
//};
//
//static void StaticBody3D_onFixedUpdate(Entity e, const StaticBody3D& body, Transform3D& tf)
//{
//    auto bodyTf = body.getTransform();
//    tf.pos      = bodyTf.pos;
//    tf.rot      = bodyTf.rot;
//}