
#pragma once

#include <TLib/EASTL.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Macros.hpp>

#define PX_PHYSX_STATIC_LIB
#include <physx/PxPhysics.h>
#include <physx/PxPhysicsAPI.h>
#include <physx/foundation/PxErrorCallback.h>
#include <physx/foundation/PxFoundation.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/characterkinematic/PxCapsuleController.h>

using namespace physx;

class TLibPhysicsAllocatorCallback : public physx::PxAllocatorCallback
{
public:
    virtual ~TLibPhysicsAllocatorCallback() = default;
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
        const String errFormat = "PhysX:\n\tFile: {}\n\tLine: {}\n\tMessage: {}";

        switch (code)
        {
            case physx::PxErrorCode::eDEBUG_INFO:
            case physx::PxErrorCode::eDEBUG_WARNING:
                tlog::debug(fmt::runtime(errFormat), file, line, message);
                break;

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

static inline TLibPhysicsAllocatorCallback physicsDefaultAllocatorCallback;
static inline TLibPhysicsErrorCallback     physicsDefaultErrorCallback;
static inline physx::PxFoundation* foundation = nullptr;
static void initPhysics()
{
    ASSERT(!foundation); // Foundation already exists
    if (foundation) { return; }
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