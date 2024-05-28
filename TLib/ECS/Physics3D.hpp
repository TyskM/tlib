
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/ECS/ECS.hpp>
#include <TLib/Physics3D.hpp>
#include <TLib/Media/Transform3D.hpp>
#include <TLib/Containers/Variant.hpp>

template <auto fn>
struct Deleter
{
    // Thanks Justin
    // https://stackoverflow.com/questions/19053351/how-do-i-use-a-custom-deleter-with-a-stdunique-ptr-member
    template <typename T>
    constexpr void operator()(T* arg) const
    { fn(arg); }
};

struct Physics3DWorld : NonAssignable
{
private:
    // PxPhysics is a singleton, don't need to free it.
    Physics3D::PxPhysics* phys  = nullptr;
    Physics3D::PxScene*   scene = nullptr;

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

    void simulate(float delta)
    {
        scene->simulate    (delta);
        scene->fetchResults(true);
    }

    Physics3D::PxRigidDynamic* createRigidBody(const Vector3f& pos = {0, 0, 0})
    {
        return phys->createRigidDynamic({ pos.x, pos.y, pos.z });
    }

    static void freeRigidBody(Physics3D::PxRigidDynamic* body)
    { if(body) body->release(); }
};

struct BoxCollider3D
{
    Physics3D::PxShape* shape = nullptr;
};

struct RigidBody3D // Non Copyable
{
private:
    using Body    = Physics3D::PxRigidDynamic;
    using BodyPtr = UPtr<Body, Deleter<Physics3DWorld::freeRigidBody>>;
    BodyPtr body {nullptr};

    using Colliders = Variant<BoxCollider3D>;

    Vector<Colliders> colliders;

public:
    RigidBody3D(Physics3DWorld& world, const Vector3f& pos = { 0, 0, 0 })
    {
        body.reset(world.createRigidBody(pos));
    }

    void addBoxCollider()
    {

    }

    friend void RigidBody3D_onFixedUpdate(Entity e, const RigidBody3D& body, Transform3D& tf);
};

static void RigidBody3D_onFixedUpdate(Entity e, const RigidBody3D& body, Transform3D& tf)
{
    auto pxTf = body.body->getGlobalPose();
    tf.pos    = Vector3f(pxTf.p.x, pxTf.p.y, pxTf.p.z);
    tf.rot    = Quat(pxTf.q.w, pxTf.q.x, pxTf.q.y, pxTf.q.z);
}
