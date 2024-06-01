
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

#include <bullet/btBulletDynamicsCommon.h>

Vector3f fromBlVec3(const btVector3& bv)
{ return Vector3f(bv.getX(), bv.getY(), bv.getZ()); }

btVector3 toBlVec3(const Vector3f& v)
{ return btVector3(v.x, v.y, v.z); }

struct BodyTransform3D
{
    Vector3f pos;
    Quat     rot;
};

struct Physics3DWorld : NonAssignable
{
public:
    btDiscreteDynamicsWorld* phys;

    btAlignedObjectArray<btCollisionShape*> collisionShapes;

    struct DebugRender : btIDebugDraw
    {
        Vector<Vector3f> lines;

        virtual void clearLines() override
        {
            lines.clear();
        }

        virtual void flushLines() override
        {
            Renderer3D::drawLines(lines, ColorRGBAf::white(), GLDrawMode::Lines);
        }

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& blcolor) override
        {
            lines.emplace_back(from.getX(), from.getY(), from.getZ());
            lines.emplace_back(  to.getX(),   to.getY(),   to.getZ());
        }

        virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
        { }

        virtual void reportErrorWarning(const char* warningString) override
        { tlog::error(warningString); }

        virtual void draw3dText(const btVector3& location, const char* textString) override
        { }

        virtual void setDebugMode(int debugMode) override
        { }

        virtual int getDebugMode() const override
        { return DBG_DrawWireframe; }
    } debugRender;

public:

    ~Physics3DWorld()
    { reset(); }

    void init()
    {
        reset();
    
        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        ASSERT(collisionConfiguration);

        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
        ASSERT(dispatcher);

        btDbvtBroadphase* broadphase = new btDbvtBroadphase();
        ASSERT(broadphase);

        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
        ASSERT(solver);

        phys = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        ASSERT(phys);

        phys->setDebugDrawer(&debugRender);
    }

    void reset()
    {

    }

    bool inited() const
    {
        return true;
    }

    void setGravity(const Vector3f& gravity)
    {
        phys->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }

    Vector3f getGravity() const
    {
        auto grav = phys->getGravity();
        return Vector3f(grav.x(), grav.y(), grav.z());
    }

    void simulate(float delta)
    {
        phys->stepSimulation(btScalar(delta), 1, delta);
    }

    void debugDraw()
    {
        phys->debugDrawWorld();
    }
};

class Physics3DBody
{
    UPtr<btRigidBody>     body   { nullptr };
    UPtr<btCompoundShape> shapes { nullptr };
    Physics3DWorld*       world  = nullptr;
    float                 mass   = 0.f;

public:
    void init(Physics3DWorld& world)
    {
        this->world = &world;
        shapes = makeUnique<btCompoundShape>();
    }

    bool inited() const { return world; }

    void reset()
    {
        //if (world && body) world->scene->removeActor(*body);
        //body.reset();
        //_enabled = false;
    }

    void makeRigidBody(float mass = 1.f)
    {
        this->mass = mass;
    }

    // Equivalent to setting makeRigidBody(0)
    void makeStaticBody()
    {
        mass = 0.f;
    }

    #pragma region Colliders

    void addBoxCollider(const Vector3f& halfExtents = { 1.f, 1.f, 1.f })
    {
        btCollisionShape* shape
            = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, nullptr, shape);

        if (body)
        { world->phys->removeRigidBody(body.get()); }
        body.reset(new btRigidBody(rbInfo));
        ASSERT(body);
        world->phys->addRigidBody(body.get());
    }

    // Adds all submeshes as separate colliders
    void addTriMeshCollider(const MeshData& mesh)
    {
        for (auto& subMesh : mesh.subMeshes)
        { addTriMeshCollider(subMesh); }
    }

    void addTriMeshCollider(const MeshData::SubMesh& submesh)
    {
        auto vertices = new Vector<Vector3f>();
        vertices->reserve(submesh.vertices.size());
        for (auto& vert : submesh.vertices)
        { vertices->emplace_back(vert.position); }

        auto indices = new Vector<uint32_t>(submesh.indices);

        btIndexedMesh* blMesh = new btIndexedMesh();
        blMesh->m_vertexType          = PHY_FLOAT;
        blMesh->m_numVertices         = vertices->size();
        blMesh->m_vertexStride        = sizeof(Vector3f);
        blMesh->m_vertexBase          = (unsigned char*)vertices->data();
        blMesh->m_numTriangles        = indices->size()/3;
        blMesh->m_triangleIndexStride = 3 * sizeof(uint32_t);
        blMesh->m_triangleIndexBase   = (unsigned char*)indices->data();

        btTriangleIndexVertexArray* pointless = new btTriangleIndexVertexArray();
        pointless->addIndexedMesh(*blMesh, PHY_INTEGER);
        btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(pointless, false);

        btTransform tf;
        tf.setIdentity();
        tf.setOrigin(btVector3(0, 0, 0));
        shapes->addChildShape(tf, shape);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, nullptr, shapes.get());

        if (body)
        { world->phys->removeRigidBody(body.get()); }
        body.reset(new btRigidBody(rbInfo));
        ASSERT(body);
        world->phys->addRigidBody(body.get());

        //blMesh->preallocateIndices(submesh.indices.size());
        //blMesh->preallocateVertices(submesh.vertices.size());
        //for (size_t i = 0; i < submesh.indices.size(); i += 3)
        //{
        //    btVector3 a = toBlVec3(submesh.vertices[submesh.indices[i    ]].position);
        //    btVector3 b = toBlVec3(submesh.vertices[submesh.indices[i + 1]].position);
        //    btVector3 c = toBlVec3(submesh.vertices[submesh.indices[i + 2]].position);
        //
        //    blMesh->addTriangle(a, b, c);
        //}



        //// https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/Geometry.html
        //using namespace Physics3D;
        //
        //// Create Geometry
        //PxTriangleMesh* triMesh = nullptr;
        //{
        //    static Vector<Vector3f> vertices;
        //    ASSERT(vertices.empty());
        //    vertices.reserve(submesh.vertices.size());
        //    for (auto& vert : submesh.vertices)
        //    { vertices.emplace_back(vert.position); }
        //
        //    PxTriangleMeshDesc meshDesc;
        //    meshDesc.points.count     = vertices.size();;
        //    meshDesc.points.stride    = sizeof(Vector3f);
        //    meshDesc.points.data      = vertices.data();
        //    meshDesc.triangles.count  = submesh.indices.size()/3;
        //    meshDesc.triangles.stride = 3*sizeof(uint32_t);
        //    meshDesc.triangles.data   = submesh.indices.data();
        //    ASSERT(meshDesc.isValid());
        //
        //    PxCookingParams params(world->phys->getTolerancesScale());
        //    //ASSERT(PxValidateTriangleMesh(params, meshDesc));
        //
        //    triMesh = PxCreateTriangleMesh(params, meshDesc);
        //    vertices.clear();
        //}
        //PxMeshScale triScale(physx::PxVec3(1.0f, 1.0f, 1.0f), physx::PxQuat(physx::PxIdentity));
        //PxTriangleMeshGeometry geom(triMesh, triScale);
        //
        //// Attach Geometry
        //auto shape = world->phys->createShape(geom, *world->defaultMaterial, true);
        //body->attachShape(*shape);
        //shape->release();
    }

    #pragma endregion

    void setPosition(const Vector3f& pos)
    {
        auto tf = getTransform();
        tf.pos  = pos;
        setTransform(tf);
    }

    Vector3f getPosition() const
    { return getTransform().pos; }

    BodyTransform3D getTransform() const
    {
        BodyTransform3D ret;
        const auto& pos = body->getWorldTransform().getOrigin();
        const auto& rot = body->getWorldTransform().getRotation();
        ret.pos = Vector3f(pos.getX(), pos.getY(), pos.getZ());
        ret.rot = Quat    (rot.getW(), rot.getX(), rot.getY(), rot.getZ());
        return ret;
    }

    void setTransform(const BodyTransform3D& tf)
    {
        btTransform newTf;
        newTf.setOrigin(btVector3(tf.pos.x, tf.pos.y, tf.pos.z));
        newTf.setRotation(btQuaternion(tf.rot.x, tf.rot.y, tf.rot.z, tf.rot.w));
        body->setWorldTransform(newTf);
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