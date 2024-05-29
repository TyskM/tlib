
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/ECS/ECS.hpp>
#include <TLib/Physics3D.hpp>
#include <TLib/Media/Transform3D.hpp>
#include <TLib/Containers/Variant.hpp>

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

    Physics3D::PxRigidDynamic* createRigidBody(const Vector3f& pos = {0, 0, 0})
    {
        return phys->createRigidDynamic({ pos.x, pos.y, pos.z });
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

struct BoxCollider3D
{
    Physics3D::Shape shape { nullptr };

    BoxCollider3D(Physics3DWorld& world, const Vector3f halfExtents)
    {
        Physics3D::PxBoxGeometry geom(halfExtents.x, halfExtents.y, halfExtents.z);
        shape.reset(world.createShape(geom, world.defaultMaterial));
    }
};

using Collider = Variant<BoxCollider3D>;

struct RigidBody3D // Non Copyable
{
private:
    Physics3D::RigidBody body  { nullptr };
    Vector<Collider>     colliders;
    Physics3DWorld*      world = nullptr;

public:
    RigidBody3D(Physics3DWorld& world, const Vector3f& pos = { 0, 0, 0 })
    {
        this->world = &world;
        body.reset(world.createRigidBody(pos));
        world.scene->addActor(*body);
    }

    void setPosition(const Vector3f& pos)
    {
        Physics3D::PxTransform tf = body->getGlobalPose();
        tf.p = { pos.x, pos.y, pos.z };
        body->setGlobalPose(tf);
    }

    void addBoxCollider(const Vector3f& halfExtents = {1.f, 1.f, 1.f})
    {
        auto& collider = colliders.emplace_back(BoxCollider3D(*world, halfExtents));
        body->attachShape(*std::get<BoxCollider3D>(collider).shape.get());
    }

    friend void RigidBody3D_onFixedUpdate(Entity e, const RigidBody3D& body, Transform3D& tf);
};

static void RigidBody3D_onFixedUpdate(Entity e, const RigidBody3D& body, Transform3D& tf)
{
    auto pxTf = body.body->getGlobalPose();
    tf.pos    = Vector3f(pxTf.p.x, pxTf.p.y, pxTf.p.z);
    tf.rot    = Quat(pxTf.q.w, pxTf.q.x, pxTf.q.y, pxTf.q.z);
}
