

#pragma once

#include <TLib/Media/Resource/MeshData.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Resource/GPUVertexData.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Containers/Array.hpp>

// DONE: Optimize VAO usage. // Done, thanks aiProcess_OptimizeGraph
// DONE: Add lighting
// DONE: Support materials
// TODO: Use single global VAO, VBO, and EBO to avoid the overhead of switching them.
struct Mesh : NonCopyable
{
private:
    struct MeshTexture
    {
        TextureType type = TextureType::Diffuse;
        Texture     texture;
    };

    struct Material
    {
        // TODO: why is this a UPtr?
        Array<Texture, (size_t)TextureType::Count> textures;
    };

    struct SubMesh
    {
        UPtr<GPUVertexData> vertices;
        Material            material;
    };

    Vector<SubMesh> meshes;

public:
    Vector<SubMesh>& getMeshes()
    { return meshes; }

    bool loadFromMemory(const MeshData& cpuMesh)
    {
        for (auto& cpuSubMesh : cpuMesh.subMeshes)
        {
            auto& gpuSubMesh = meshes.emplace_back();
            gpuSubMesh.vertices  = makeUnique<GPUVertexData>();
            gpuSubMesh.vertices->setLayout({ TLib::Layout::Vec3f(), TLib::Layout::Vec3f(), TLib::Layout::Vec2f() });
            gpuSubMesh.vertices->setData(cpuSubMesh.vertices);
            gpuSubMesh.vertices->setIndices(cpuSubMesh.indices);

            int32_t i = 0;
            for (auto& cpuTexture : cpuSubMesh.material.textures)
            {
                ASSERT(cpuTexture); // Textures should never be NULL, worst case they are 1x1 pure white/black/gray
                auto& gpuTexture = gpuSubMesh.material.textures[i];
                gpuTexture.create();
                gpuTexture.setData(*cpuTexture);
                gpuTexture.setUVMode(UVMode::Repeat);
                gpuTexture.setFilter(TextureMinFilter::LinearMipmapLinear, TextureMagFilter::Linear);
                gpuTexture.generateMipmaps();
                ++i;
            }
        }

        return true;
    }

    // Returns false on failure
    bool loadFromFile(const Path& path)
    {
        MeshData meshData;
        bool success = meshData.loadFromFile(path);
        if (!success) { tlog::error("Model::loadFromFile: Failed to load mesh"); return false; }
        ASSERT(success);

        reset();
        return loadFromMemory(meshData);
    }

    void reset()
    {
        meshes.clear();
    }
};
