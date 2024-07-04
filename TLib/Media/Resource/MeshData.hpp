#pragma once

#include <TLib/Types/Types.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Media/Resource/TextureData.hpp>
#include <TLib/Files.hpp>
#include <TLib/Containers/Stack.hpp>
#include <TLib/Containers/Array.hpp>

#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex
{
    Vector3f position;
    Vector3f normal;
    Vector2f texCoords;
};

struct SkeletonVertex
{
    Vector3f position;
    Vector3f normal;
    Vector2f texCoords;
    //Vector3f tangent;
    //Vector3f bitangent;

    static constexpr uint32_t maxBoneInfluences = 4;
    Array<uint32_t, maxBoneInfluences> boneIds;
    Array<float,    maxBoneInfluences> boneWeights;
};

enum class TextureType
{
    Diffuse,   // = aiTextureType::aiTextureType_DIFFUSE,
    Roughness, // = aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS
    Metalness, // = aiTextureType::aiTextureType_METALNESS,
    // TODO: Support normal maps

    Count,
    None       // = aiTextureType::aiTextureType_NONE,
};

struct MeshData
{
    struct MaterialTexture
    {
        TextureType       type = TextureType::None;
        UPtr<TextureData> textureData;
    };

    struct Material
    {
        String name = "???";
        // Texture should not be NULL
        Array<UPtr<TextureData>, (size_t)TextureType::Count> textures;
    };

    struct SubMesh
    {
        String           name;
        Vector<Vertex>   vertices;
        Vector<uint32_t> indices;
        Material         material;
    };

    struct Animation
    {
        template <typename T>
        struct Key
        {
            double time = 0.f;
            T      value;
        };

        struct Channel
        {
            String                name;
            Vector<Key<Vector3f>> positions;
            Vector<Key<Quat>>     rotations;
            Vector<Key<Vector3f>> scales;
        };

        double          duration       = 0.f;
        double          ticksPerSecond = 0.f;
        Vector<Channel> channels;
    };

    Path              path;
    Vector<SubMesh>   subMeshes;
    Vector<Animation> animations;

    bool loadFromFile(const Path& path)
    {
        // https://learnopengl.com/Model-Loading/Model
        this->path = path;

        const aiPostProcessSteps importerFlags =
            aiProcess_Triangulate           | aiProcess_GenNormals    |
            aiProcess_OptimizeMeshes        | aiProcess_OptimizeGraph |
            aiProcess_JoinIdenticalVertices | // TODO: Read somewhere this flag could cause problems, maybe look into it.
            aiProcess_FlipUVs; // Renderer class already handles the OpenGL conversion.

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(), importerFlags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            tlog::error("MeshData::loadFromFile: Failed to load mesh\nReason: {}\nPath: {}\nWorking Directory: {}",
                importer.GetErrorString(), path.string(), fs::current_path().string());
            return false;
        }

        reset();

        Path fileFolder = Path(path).remove_filename();

        static Stack<aiNode*> stack;
        stack.get_container().clear();
        stack.push(scene->mRootNode);

        // Load meshes
        while (!stack.empty())
        {
            aiNode* node = stack.top();
            stack.pop();

            if (scene->HasMeshes())
            {
                uint32_t meshCount = node->mNumMeshes;
                for (uint32_t i = 0; i < meshCount; i++)
                {
                    auto& subMesh  = subMeshes.emplace_back();
                    auto& vertices = subMesh.vertices;
                    auto& indices  = subMesh.indices;

                    aiMesh*  mesh         = scene->mMeshes[node->mMeshes[i]];
                    uint32_t verticeCount = mesh->mNumVertices;
                    uint32_t faceCount    = mesh->mNumFaces;
                    auto     texCoords    = mesh->mTextureCoords[0];
                    subMesh.name          = mesh->mName.C_Str();

                    // process vertex positions, normals and texture coordinates
                    ASSERT(vertices.empty()); // sanity check because im a fuck up
                    vertices.reserve(verticeCount);
                    for (uint32_t i = 0; i < verticeCount; i++)
                    {
                        Vertex& vertex = subMesh.vertices.emplace_back();

                        auto& pos = mesh->mVertices[i];
                        vertex.position = Vector3f(pos.x, pos.y, pos.z);

                        auto& normal = mesh->mNormals[i];
                        vertex.normal = Vector3f(normal.x, normal.y, normal.z);

                        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                        { vertex.texCoords ={ texCoords[i].x, texCoords[i].y }; }
                        else
                        { vertex.texCoords = Vector2f(0.0f, 0.0f); }
                    }

                    // process indices
                    ASSERT(indices.empty());
                    indices.reserve(faceCount * 3);
                    for (uint32_t i = 0; i < faceCount; i++)
                    {
                        aiFace& face        = mesh->mFaces[i];
                        uint32_t indiceCount = face.mNumIndices;
                        for (uint32_t j = 0; j < indiceCount; j++)
                        { indices.push_back(face.mIndices[j]); }
                    }

                    // process material
                    if (mesh->mMaterialIndex >= 0)
                    {
                        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                        loadMaterialTextures(subMesh.material, scene, material, fileFolder);
                    }
                }
            }

            // Queue children for above process
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            { stack.push(node->mChildren[i]); }
        }

        if (scene->HasAnimations())
        {
            uint32_t animCount = scene->mNumAnimations;
            animations.reserve(animCount);
            for (uint32_t i = 0; i < animCount; i++)
            {
                auto& anim   = animations.emplace_back();
                auto& aianim = scene->mAnimations[i];

                anim.duration       = aianim->mDuration;
                anim.ticksPerSecond = aianim->mTicksPerSecond;

                uint32_t channelCount = aianim->mNumChannels;
                anim.channels.reserve(channelCount);
                for (uint32_t j = 0; j < channelCount; j++)
                {
                    auto& channel   = anim.channels.emplace_back();
                    auto& aichannel = aianim->mChannels[j];
                    
                    channel.name = aichannel->mNodeName.C_Str();

                    uint32_t posKeyCount = aichannel->mNumPositionKeys;
                    channel.positions.reserve(posKeyCount);
                    for (uint32_t k = 0; k < posKeyCount; k++)
                    {
                        auto& pos   = channel.positions.emplace_back();
                        auto& aipos = aichannel->mPositionKeys[k];
                        pos.time    = aipos.mTime;
                        pos.value   = Vector3f(aipos.mValue.x, aipos.mValue.y, aipos.mValue.z);
                    }

                    uint32_t rotKeyCount = aichannel->mNumRotationKeys;
                    channel.rotations.reserve(rotKeyCount);
                    for (uint32_t k = 0; k < rotKeyCount; k++)
                    {
                        auto& pos   = channel.rotations.emplace_back();
                        auto& airot = aichannel->mRotationKeys[k];
                        pos.time    = airot.mTime;
                        pos.value   = Quat(airot.mValue.w, airot.mValue.x, airot.mValue.y, airot.mValue.z);
                    }

                    uint32_t sclKeyCount = aichannel->mNumScalingKeys;
                    channel.scales.reserve(sclKeyCount);
                    for (uint32_t k = 0; k < sclKeyCount; k++)
                    {
                        auto& pos   = channel.scales.emplace_back();
                        auto& aiscl = aichannel->mScalingKeys[k];
                        pos.time    = aiscl.mTime;
                        pos.value   = Vector3f(aiscl.mValue.x, aiscl.mValue.y, aiscl.mValue.z);
                    }
                }
            }
        }

        return true;
    }

    void reset()
    {
        subMeshes.clear();
        animations.clear();
    }

private:
    static inline uint8_t fallbackTexture[4] = {255, 0, 255, 255};

    void loadMaterialTexture(const aiString& aipath, const ColorRGBAf& fallbackColor, TextureType texType, const aiScene* scene, const Path& modelPath, Material& outMat)
    {
        const bool missingTexture = aipath.C_Str()[0] == '\0';
        auto& outTex              = outMat.textures[(int32_t)texType];
        outTex                    = makeUnique<TextureData>();

        // FALLBACK TEXTURE
        if (missingTexture)
        {
            ColorRGBAi fallbackColorInt = fallbackColor;
            outTex->loadFromMemoryRaw((uint8_t*)&fallbackColorInt, 1, 1, 4);
        }

        // EMBEDDED TEXTURE
        else if (const aiTexture* texture = scene->GetEmbeddedTexture(aipath.C_Str()))
        {
            // TEXTURE IS PNG/JPEG
            if (texture->mHeight == 0)
            {
                bool success = outTex->loadFromMemory((uint8_t*)texture->pcData, texture->mWidth);
                if (!success)
                { tlog::error("loadMaterialTextures (EMBEDDED::PNG/JPEG): Failed to model texture '{}'", modelPath.string()); }
            }

            // TEXTURE IS RAW RGB
            else
            {
                bool success = outTex->loadFromMemory((uint8_t*)texture->pcData, texture->mWidth * texture->mHeight);
                if (!success)
                { tlog::error("loadMaterialTextures (EMBEDDED::RAW): Failed to model texture '{}'", modelPath.string()); }
            }
        }

        // TEXTURE IN FILESYSTEM
        else
        {
            Path texPath = (modelPath / aipath.C_Str());
            outTex->loadFromPath(texPath);
        }
    }

    void loadMaterialTextures(Material& mat, const aiScene* scene, aiMaterial* aimat, const Path& modelPath)
    {
        mat.name = aimat->GetName().C_Str();
        auto& textures = mat.textures;

        {   // Load Diffuse
            aiString  diffusePath;
            aiColor3D fallbackDiffuse(0.f, 0.f, 0.f);
            aimat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &diffusePath);
            aimat->Get(AI_MATKEY_COLOR_DIFFUSE, fallbackDiffuse);
            ColorRGBAf diffuseColor (fallbackDiffuse.r, fallbackDiffuse.g, fallbackDiffuse.b, 1.f);
            loadMaterialTexture(diffusePath, diffuseColor, TextureType::Diffuse, scene, modelPath, mat);
            ASSERT(mat.textures[(int)TextureType::Diffuse]);
        }

        {   // Load Roughness
            aiString roughnessPath;
            float    fallbackRoughness = 1.f;
            aimat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughnessPath);
            aimat->Get(AI_MATKEY_ROUGHNESS_FACTOR, fallbackRoughness);
            ColorRGBAf roughnessColor (fallbackRoughness, fallbackRoughness, fallbackRoughness, 1.f);
            loadMaterialTexture(roughnessPath, roughnessColor, TextureType::Roughness, scene, modelPath, mat);
            ASSERT(mat.textures[(int)TextureType::Roughness]);
        }

        {   // Load Metallic
            aiString metallicPath;
            float    fallbackMetallic = 0.f;
            aimat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallicPath);
            aimat->Get(AI_MATKEY_METALLIC_FACTOR, fallbackMetallic);
            ColorRGBAf metallicColor (fallbackMetallic, fallbackMetallic, fallbackMetallic, 1.f);
            loadMaterialTexture(metallicPath, metallicColor, TextureType::Metalness, scene, modelPath, mat);
            ASSERT(mat.textures[(int)TextureType::Metalness]);
        }
    }
};
