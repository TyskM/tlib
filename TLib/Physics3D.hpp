
#pragma once

#include <TLib/EASTL.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Types/Types.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Media/Renderer3D.hpp>

#include <physx/PxPhysics.h>
#include <physx/PxPhysicsAPI.h>
#include <physx/foundation/PxErrorCallback.h>
#include <physx/foundation/PxFoundation.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/characterkinematic/PxCapsuleController.h>
#include <physx/extensions/PxDefaultAllocator.h>

static inline physx::PxFoundation* physics3dFoundation = nullptr;
static inline physx::PxPhysics*    physics3d           = nullptr;

namespace Phys3D
{
    using namespace physx;

    namespace Detail
    {
        class TLibPhysicsAllocatorCallback : public physx::PxAllocatorCallback
        {
            // Allocations MUST be 16 byte aligned.
            // https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/API.html

        public:
            virtual ~TLibPhysicsAllocatorCallback() = default;
            virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
            { return mi_malloc_aligned(size, 16); }
            virtual void deallocate(void* ptr)
            { return mi_free_aligned(ptr, 16); }
        };

        class TLibPhysicsErrorCallback : public physx::PxErrorCallback
        {
        public:
            virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
            {
                const String errFormat = "PhysX:\n\tFile: {}\n\tLine: {}\n\tMessage: {}";

                switch (code)
                {
                    case physx::PxErrorCode::eDEBUG_INFO:
                        tlog::warn(fmt::runtime(errFormat), file, line, message);
                        break;

                    case physx::PxErrorCode::eDEBUG_WARNING:
                    case physx::PxErrorCode::ePERF_WARNING:
                        tlog::warn(fmt::runtime(errFormat), file, line, message);
                        break;

                    case physx::PxErrorCode::eNO_ERROR:
                    case physx::PxErrorCode::eINVALID_PARAMETER:
                    case physx::PxErrorCode::eINVALID_OPERATION:
                    case physx::PxErrorCode::eOUT_OF_MEMORY:
                    case physx::PxErrorCode::eINTERNAL_ERROR:
                    case physx::PxErrorCode::eABORT:
                    case physx::PxErrorCode::eMASK_ALL:
                    default:
                        tlog::error(fmt::runtime(errFormat), file, line, message);
                        break;
                }
            }
        };

        //static inline TLibPhysicsAllocatorCallback physicsDefaultAllocatorCallback;
        static inline physx::PhysicsErrorCallback  physicsDefaultAllocatorCallback;
        static inline TLibPhysicsErrorCallback     physicsDefaultErrorCallback;
    }

    // This makes physics3dFoundation and physics3d valid
    void init()
    {
        ASSERT(!physics3dFoundation); // Foundation already exists
        if (physics3dFoundation) { return; }
        physics3dFoundation = PxCreateFoundation(PX_PHYSICS_VERSION,
            Detail::physicsDefaultAllocatorCallback,
            Detail::physicsDefaultErrorCallback);
        ASSERT(physics3dFoundation); // Failed to create foundation

        physx::PxTolerancesScale scale{};
        bool recordMemoryAllocations = false;

        physics3d = PxCreatePhysics(PX_PHYSICS_VERSION, *physics3dFoundation, scale, recordMemoryAllocations, NULL);
        ASSERT(physics3d);

        if (!PxInitExtensions(*physics3d, NULL))
        { tlog::error("PxInitExtensions failed"); ASSERT(false); }
    }

    static void shutdown()
    {
        ASSERT(physics3dFoundation);
        if (physics3dFoundation)
        {
            physics3dFoundation->release();
            physics3dFoundation = nullptr;
        }

        if (physics3d)
        {
            physics3d->release();
            physics3d = nullptr;
        }
    }

    physx::PxScene* createScene()
    {
        physx::PxSceneDesc desc(physics3d->getTolerancesScale());
        desc.filterShader  = physx::PxDefaultSimulationFilterShader;
        desc.gravity       = { 0.f, -20.f, 0.f };
        desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
        ASSERT(desc.isValid());
        physx::PxScene* physicsScene = physics3d->createScene(desc);
        ASSERT(physicsScene);
        return physicsScene;
    }

    using Material   = UPtr<PxMaterial,     Deleter< [](PxMaterial*     mat)   { mat  ->release(); } >>;
    using Shape      = UPtr<PxShape,        Deleter< [](PxShape*        shape) { shape->release(); } >>;
    using RigidBody  = UPtr<PxRigidDynamic, Deleter< [](PxRigidDynamic* body)  { body ->release(); } >>;
    using StaticBody = UPtr<PxRigidStatic,  Deleter< [](PxRigidStatic*  body)  { body ->release(); } >>;
    using BodyBase   = UPtr<PxRigidActor,   Deleter< [](PxRigidActor*   body)  { body ->release(); } >>;
    
    using Geometry        = PxGeometry;
    using TriMeshGeometry = PxTriangleMeshGeometry;
    using BoxGeometry     = PxBoxGeometry;
    using SphereGeometry  = PxSphereGeometry;

    enum class MaterialCombine
    {
        Minimum,
        Average,
        Maximum,
        Multiply
    };

    enum class Axis
    {
        X = bit(0),
        Y = bit(1),
        Z = bit(2)
    };

    struct BodyTransform3D
    {
        Vector3f pos;
        Quat     rot;

        Transform3D toTf3d() const
        { return Transform3D(pos, Vector3f(1, 1, 1), rot); }
    };

    Material createMaterial(float staticFriction = 0.9f, float dynamicFriction = 0.9f, float restitution = 0.1f)
    {
        return Material(physics3d->createMaterial(staticFriction, dynamicFriction, restitution));
    }

    Shape createShape(
        const PxGeometry& geometry,
        const Material&   material = createMaterial())
    {
        return Shape(physics3d->createShape(geometry, *material, false));
    }

    static Vector3f fromPxVec3(const Phys3D::PxVec3& v)
    {
        return Vector3f(v.x, v.y, v.z);
    }

    static Quat fromPxQuat(const Phys3D::PxQuat& q)
    {
        return Quat(q.w, q.x, q.y, q.z);
    }

    static BodyTransform3D fromPxTf(const Phys3D::PxTransform& tf)
    {
        BodyTransform3D ret;
        ret.pos = fromPxVec3(tf.p);
        ret.rot = fromPxQuat(tf.q);
        return ret;
    }

    static Phys3D::PxVec3 toPxVec3(const Vector3f& v)
    { return Phys3D::PxVec3(v.x, v.y, v.z); }

    static Phys3D::PxQuat toPxQuat(const Quat& q)
    {
        return Phys3D::PxQuat(q.x, q.y, q.z, q.w);
    }

    static BoxGeometry createBoxGeometry(const Vector3f& halfExtents = { 1.f, 1.f, 1.f })
    {
        return PxBoxGeometry(halfExtents.x, halfExtents.y, halfExtents.z);
    }

    static TriMeshGeometry createTriMeshGeometry(const MeshData::SubMesh& submesh)
    {
        // https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/Geometry.html

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

            PxCookingParams params(physics3d->getTolerancesScale());

            triMesh = PxCreateTriangleMesh(params, meshDesc);
            vertices.clear();
        }
        PxMeshScale triScale(physx::PxVec3(1.0f, 1.0f, 1.0f), physx::PxQuat(physx::PxIdentity));
        PxTriangleMeshGeometry geom(triMesh, triScale);
        return geom;
    }

    static Vector<TriMeshGeometry> createTriMeshGeometry(const MeshData& mesh)
    {
        auto size = mesh.subMeshes.size();
        Vector<TriMeshGeometry> ret(size);
        for (size_t i = 0; i < size; i++)
        {
            auto& subMesh = mesh.subMeshes[i];
            ret[i] = createTriMeshGeometry(subMesh);
        }
        return ret;
    }

    struct Collider
    {
        Shape collider { nullptr };

        void set(const Geometry& geometry, const Material& material = createMaterial())
        {
            collider = createShape(geometry, material);
        }

        void setFilterGroup(uint32_t group)
        {
            PxFilterData filter;
            filter.word0 = group;
            collider->setQueryFilterData(filter);
        }

        uint32_t getFilterGroup() const
        { return collider->getQueryFilterData().word0; }
    };

    struct Scene;
    struct Body
    {
        BodyBase body { nullptr };

              Scene* scene()       { return (      Scene*)body->getScene()->userData; }
        const Scene* scene() const { return (const Scene*)body->getScene()->userData; }

        void reset()
        {
            body.reset();
        }

        void makeRigidBody(const Vector3f& pos ={ 0, 0, 0 })
        {
            body.reset(physics3d->createRigidDynamic({ pos.x, pos.y, pos.z }));
            body->userData = this;
        }

        void makeStaticBody(const Vector3f& pos ={ 0, 0, 0 })
        {
            body.reset(physics3d->createRigidStatic({ pos.x, pos.y, pos.z }));
            body->userData = this;
        }

        void addCollider(Collider& collider)
        {
            body->attachShape(*collider.collider);
        }

        #pragma region Transform

        void setPosition(const Vector3f& pos, bool wake = true)
        {
            PxTransform tf = body->getGlobalPose();
            tf.p = { pos.x, pos.y, pos.z };
            body->setGlobalPose(tf, wake);
        }

        Vector3f getPosition() const
        {
            PxTransform tf = body->getGlobalPose();
            return Vector3f(tf.p.x, tf.p.y, tf.p.z);
        }

        void setTransform(const Vector3f& pos, const Quat& rot, bool wake = true)
        {
            PxTransform tf(toPxVec3(pos), toPxQuat(rot));
            body->setGlobalPose(tf, wake);
        }

        BodyTransform3D getTransform() const
        {
            BodyTransform3D ret;
            PxTransform tf = body->getGlobalPose();
            ret.pos = Vector3f(tf.p.x, tf.p.y, tf.p.z);
            ret.rot = Quat(tf.q.w, tf.q.x, tf.q.y, tf.q.z);
            return ret;
        }

        #pragma endregion

        #pragma region Dynamic Bodies

        bool isDynamic() const
        { return body->is<PxRigidDynamic>(); }

        void setMass(float mass)
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            { dynBody->setMass(mass); }
            else
            { tlog::warn("setMass(): Target is not dynamic"); }
        }

        void setAxisRotLock(Axis axis)
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            {
                dynBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, hasFlag(axis, Axis::X));
                dynBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, hasFlag(axis, Axis::Y));
                dynBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, hasFlag(axis, Axis::Z));
            }
            else
            { tlog::warn("setAxisRotLock(): Target is not dynamic"); }
        }

        void setLinearVelocity(const Vector3f& vel, bool wake = true)
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            { dynBody->setLinearVelocity(toPxVec3(vel), wake); }
            else
            { tlog::warn("setLinearVelocity(): Target is not dynamic"); }
        }

        Vector3f getLinearVelocity() const
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            { return fromPxVec3(dynBody->getLinearVelocity()); }
            else
            { tlog::warn("getLinearVelocity(): Target is not dynamic"); }
        }

        void setAngularVelocity(const Vector3f& vel, bool wake = true)
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            { dynBody->setAngularVelocity(toPxVec3(vel), wake); }
            else
            { tlog::warn("setAngularVelocity(): Target is not dynamic"); }
        }

        Vector3f getAngularVelocity() const
        {
            if (auto dynBody = body->is<PxRigidDynamic>())
            { return fromPxVec3(dynBody->getAngularVelocity()); }
            else
            { tlog::warn("getAngularVelocity(): Target is not dynamic"); }
        }

        #pragma endregion
    };

    struct Scene : NonAssignable
    {
    public:
        Phys3D::PxScene* scene = nullptr;

    public:
        ~Scene()
        { reset(); }

        void init(const Vector3f& gravity = { 0.f, -20.f, 0.f },
                  PxSimulationEventCallback* simCallback = NULL,
                  PxSimulationFilterShader   simShader   = PxDefaultSimulationFilterShader)
        {
            using namespace Phys3D;
            reset();

            if (!physics3d) { Phys3D::init(); }

            physx::PxSceneDesc desc(physics3d->getTolerancesScale());
            desc.filterShader  = simShader;
            desc.gravity       = toPxVec3(gravity);
            desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
            desc.simulationEventCallback = simCallback;
            ASSERT(desc.isValid());

            scene = physics3d->createScene(desc);
            ASSERT(scene);

            scene->userData = this;
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

        void addBody(Body& body)
        {
            scene->addActor(*body.body);
        }

        void removeBody(Body& body, bool wakeOnLostTouch = false)
        {
            scene->removeActor(*body.body, wakeOnLostTouch);
        }

        void setGravity(const Vector3f& gravity)
        { scene->setGravity(PxVec3(gravity.x, gravity.y, gravity.z)); }

        Vector3f getGravity() const
        { return Vector3f(scene->getGravity()); }

        // TODO: https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/BestPractices.html#performance-issues
        void simulate(float delta)
        {
            scene->simulate    (delta);
            scene->fetchResults(true);
        }

        void debugDraw()
        {
            using namespace physx;

            scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f); // Required
            scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
            scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
            scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);

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
                lines.emplace_back(line.pos0.x, line.pos0.y, line.pos0.z);
                lines.emplace_back(line.pos1.x, line.pos1.y, line.pos1.z);
            }
            if (!lines.empty())
                Renderer3D::drawLines(lines, ColorRGBAf::white(), GLDrawMode::Lines);

            static Vector<Vector3f> tris;
            tris.clear();
            tris.reserve(rb.getNbTriangles());
            for (PxU32 i=0; i < rb.getNbTriangles(); i++)
            {
                const PxDebugTriangle& tri = rb.getTriangles()[i];
                tris.emplace_back(tri.pos0.x, tri.pos0.y, tri.pos0.z);
                tris.emplace_back(tri.pos1.x, tri.pos1.y, tri.pos1.z);
                tris.emplace_back(tri.pos2.x, tri.pos2.y, tri.pos2.z);
            }
            if (!tris.empty())
                Renderer3D::drawLines(tris, ColorRGBAf::gray(), GLDrawMode::Triangles);
        }
    };
}