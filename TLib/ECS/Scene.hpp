
#pragma once
#include <TLib/ECS/ECS.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/Files.hpp>

#include <TLib/ECS/MeshInstance3D.hpp>
#include <TLib/ECS/Physics3D.hpp>

struct Scene
{
    /* Components:
        Transform3D
        MeshInstance3D

        Physics:
            RigidBody3D
    */


    ECS ecs;
    Physics3DWorld phys3d;

    System sys_MeshInstance3DRender;
    System sys_rigidBody3DFixedUpdate;

    float fixedTimeStep  = 1.f/60.f;
    float time           = 0;
    float lastUpdateTime = 0;
    float timeBuffer     = 0;

    void init()
    {
        // Setup Rendering
        sys_MeshInstance3DRender = ecs.system<MeshInstance3D, const Transform3D>("MeshInstance3D Render").each(&MeshInstance3D_onRender);
        
        // Setup physics
        phys3d.init();
        sys_rigidBody3DFixedUpdate = ecs.system<const RigidBody3D, Transform3D>("RigidBody3D Fixed Update").each(&RigidBody3D_onFixedUpdate);

        auto theHolyCube = ecs.entity();
        emplaceComponent<Transform3D>(theHolyCube, Vector3f::up() * 5.f);

        const Path theHolyCubeModel("assets/primitives/cube.obj");
        emplaceComponent<MeshInstance3D>(theHolyCube, theHolyCubeModel);

        auto& body = emplaceComponent<RigidBody3D>(theHolyCube, phys3d);
        
    }


    void fixedUpdate(float delta)
    {
        sys_rigidBody3DFixedUpdate.run(delta);
        phys3d.simulate(delta);
    }

    void update(float delta)
    {
        // Fixed update
        time          += delta;
        timeBuffer    += time - lastUpdateTime;
        lastUpdateTime = time;
        while (timeBuffer >= fixedTimeStep)
        {
            fixedUpdate(fixedTimeStep);
            timeBuffer -= fixedTimeStep;
        }
    }

    void render(float delta)
    {
        sys_MeshInstance3DRender.run(delta);
    }
};
