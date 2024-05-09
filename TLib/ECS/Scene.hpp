
#pragma once
#include <TLib/ECS/ECS.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/Files.hpp>

#include <TLib/ECS/MeshInstance3D.hpp>

struct Scene
{
    /* Components:
        Transform3D
        MeshInstance3D

        Physics:
            RigidBody3D
    */


    ECS ecs;

    System sysMeshInstance3DRender;

    float fixedTimeStep  = 1.f/60.f;
    float time           = 0;
    float lastUpdateTime = 0;
    float timeBuffer     = 0;

    #pragma region Constructors/Destructors

    void onSetRigidBody3D(Entity e, RigidBody3D& body)
    {

    }

    #pragma endregion

    void init()
    {
        sysMeshInstance3DRender = ecs.system<MeshInstance3D, const Transform3D>("MeshInstance3D Render").each(
            [&](Entity e, MeshInstance3D& mesh, const Transform3D& tf) { MeshInstance3D::onRender(e, mesh, tf); });

        auto e = ecs.entity();
        e.emplace<Transform3D>(Vector3f::up() * 5.f);

        const Path p("assets/primitives/cube.obj");
        e.emplace<MeshInstance3D>(p);
    }


    void fixedUpdate(float delta)
    {

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
        sysMeshInstance3DRender.run(delta);
    }
};
