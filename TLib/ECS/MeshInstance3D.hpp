
#pragma once
#include <TLib/ECS/ECS.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/Files.hpp>

using R3D = Renderer3D;

struct MeshInstance3D
{
    Asset<Mesh> mesh;

    MeshInstance3D(const Path& path)
    {
        tlog::info("Creating mesh instance: {}", path.string());

        if (path.empty())
        { /* TODO: Fallback mesh */ }

        mesh = new Mesh(); // TODO: Leak
        mesh->loadFromFile(path);
    }

    static void onRender(Entity e, MeshInstance3D& mesh, const Transform3D& tf)
    {
        R3D::drawModel(mesh.mesh.get(), tf);
    }
};
