
#pragma once
#include <TLib/ECS/ECS.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/Files.hpp>

#include <TLib/ECS/MeshInstance3D.hpp>
#include <TLib/ECS/Physics3D.hpp>

struct Scene
{
    Physics3DWorld phys3d;

    System sys_MeshInstance3DRender;
    System sys_Physics3DFixedUpdate;
    ECS    ecs;

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
        phys3d.setGravity({ 0.f, -20.f, 0.f });
        sys_Physics3DFixedUpdate  = ecs.system<const Physics3DBody, Transform3D>("Physics3DBody Fixed Update") .each(&Physics3DBody_onFixedUpdate);

    }

    void reset()
    {
        ecs.reset();
        phys3d.reset();
    }

    ~Scene() { reset(); }

    Entity createEntity()
    { return ecs.entity(); }

    template <typename T, typename... Args>
    T& emplaceComponent(Entity entity, Args&&... args)
    { return ::emplaceComponent<T>(entity, args...); }

    void fixedUpdate(float delta)
    {
        sys_Physics3DFixedUpdate.run(delta);
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
