
// TODO: Finish model example

#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/RNG.hpp>
#include "Common.hpp"

#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum class ViewMode
{
    Perspective,
    Orthographic
};

struct Camera3D
{
    float     znear    = 0.02f;
    float     zfar     = 3000.f;
    float     fov      = 90.f;
    glm::vec3 pos      = { 0.0f, 0.0f, 0.0f };
    glm::vec3 target   = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up       = { 0.0f, 1.0f, 0.0f };
    ViewMode  viewmode = ViewMode::Perspective;

    inline void setPos(const Vector3f& posv) { this->pos ={ posv.x, posv.y, posv.z }; }
    [[nodiscard]] inline Vector3f getPos() const { return Vector3f(pos); }

    inline void setTarget(const Vector3f& targetv) { target ={ targetv.x, targetv.y, targetv.z }; }
    [[nodiscard]] inline Vector3f getTarget() const { return Vector3f(target); }

    inline void setUp(const Vector3f& upv) { up = { upv.x, upv.y, upv.z }; }
    [[nodiscard]] inline Vector3f getUp() const { return Vector3f(up); }

    [[nodiscard]]
    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(pos, target, up);
    }

    [[nodiscard]]
    glm::mat4 getPerspectiveMatrix() const
    {
        Vector2f size = Vector2f(Renderer::getFramebufferSize());
        switch (viewmode)
        {
            case ViewMode::Perspective:
                return glm::perspective(glm::radians(fov), size.x / size.y, znear, zfar); break;
            case ViewMode::Orthographic:
                return glm::ortho(0.f, size.x, 0.f, size.y, znear, zfar); break;
            default: return glm::mat4(); break;
        }
    }
};

#pragma region Shaders
const char* vert_3d = R"""(
        #version 330 core
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 normal;
        layout (location = 2) in vec2 texCoords;

        out vec3 vertNormal;
        out vec2 vertTexCoords;
        out vec3 vertFragPos;
        out vec3 vertLightPos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 lightPos;

        void main()
        {
            gl_Position   = projection * view * model * vec4(position, 1.0);

            // https://learnopengl.com/Lighting/Basic-Lighting
            vertNormal    = mat3(transpose(inverse(view * model))) * normal;

            vertTexCoords = texCoords;
            vertFragPos   = vec3(view * model * vec4(position, 1.0));
            vertLightPos  = vec3(view * vec4(lightPos, 1.0));
        }
        )""";

const char* frag_3d = R"""(
        #version 330 core
        out vec4 fragColor;

        in vec3 vertNormal;
        in vec2 vertTexCoords;
        in vec3 vertFragPos;
        in vec3 vertLightPos;

        uniform vec3 lightColor = vec3(0.2, 0.9, 0.2);
        uniform float ambientStrength = 0.1;

        uniform sampler2D texture_diffuse;
        uniform sampler2D texture_specular;

        void main()
        {
            // Ambient Lighting
            vec3 lightAmbient = ambientStrength * lightColor;

            // Diffuse lighting
            vec3 norm = normalize(vertNormal);
            vec3 lightDir = normalize(vertLightPos - vertFragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 lightDiffuse = diff * lightColor;

            // Specular
            float specularStrength = 0.7;
            vec3 viewDir = normalize(-vertFragPos);
            vec3 reflectDir = reflect(-lightDir, vertNormal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor; 

            // Texture
            vec4 texColor = texture(texture_diffuse, vec2(vertTexCoords.x, vertTexCoords.y));

            // Result
            vec3 lightResult = lightAmbient + lightDiffuse + specular;
            fragColor = vec4(lightResult, 1.0) * texColor;
        }
        )""";
#pragma endregion

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

// TODO:
// Optimize VAO usage. One per model? // Done, thanks aiProcess_OptimizeGraph
/*

Use single global VAO, VBO, and EBO to avoid the overhead of switching them.

global VertexArray;  // Layout
global VertexBuffer; // Vertices
global ElementBuffer // Indices

size_t vboIndex, vboSize;
size_t eboIndex, eboSize;

*/
// Add lighting
// Support materials
struct Model : NonAssignable
{
public:
    using TextureType = aiTextureType;

private:
    struct MeshTexture
    {
        TextureType type = aiTextureType_NONE;
        Texture* texture = nullptr;
    };

    // TODO: move everything to one VAO wtf are you doing
    struct ModelMesh
    {
        Mesh mesh;
        Vector<MeshTexture> textures;
    };

    static inline Vector<UPtr<Texture>> texCache;
    Vector<ModelMesh> meshes;

    void loadMaterialTextures(Vector<MeshTexture>& textures, aiMaterial* mat, aiTextureType type, const fs::path& path)
    {
        size_t texCount = mat->GetTextureCount(type);
        for (unsigned int i = 0; i < texCount; i++)
        {
            aiString str;
            mat->GetTexture(type,i,&str);
            MeshTexture& newTex = textures.emplace_back();
            fs::path texPath = (path / str.C_Str()).lexically_normal();

            bool texIsCached = false;
            for (auto& cachedTex : texCache)
            {
                if (cachedTex->isFromSamePath(texPath))
                {
                    tlog::info("Dupe path");
                    texIsCached = true;
                    newTex.texture = cachedTex.get();
                    break;
                }
            }

            // Load new texture
            if (!texIsCached)
            {
                tlog::info("Loading new texture: {}", texPath.string());
                auto& tex = texCache.emplace_back(makeUnique<Texture>());
                tex->loadFromFile(texPath.string());
                tex->setUVMode(UVMode::Repeat);
                newTex.texture = tex.get();
            }

            ASSERT(newTex.texture->valid());
            newTex.type = type;
        }
    }

    void processMesh(aiMesh* aimesh, const aiScene* scene, const fs::path& path)
    {
        Vector<Vertex> vertices;
        vertices.reserve(aimesh->mNumVertices);

        Vector<uint32_t> indices;

        ModelMesh& modelMesh = meshes.emplace_back();
        modelMesh.mesh.setLayout({Layout::Vec3f(), Layout::Vec3f(), Layout::Vec2f()});

        // process vertex positions, normals and texture coordinates
        for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
        {
            Vertex vertex;

            vertex.position = glm::vec3(aimesh->mVertices[i].x,
                aimesh->mVertices[i].y,
                aimesh->mVertices[i].z);

            vertex.normal = glm::vec3(aimesh->mNormals[i].x,
                aimesh->mNormals[i].y,
                aimesh->mNormals[i].z);

            if (aimesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            { vertex.texCoords = { aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y}; } else
            { vertex.texCoords = glm::vec2(0.0f,0.0f); }

            vertices.push_back(vertex);
        }

        modelMesh.mesh.setData(vertices);

        // process indices
        for (unsigned int i = 0; i < aimesh->mNumFaces; i++)
        {
            aiFace face = aimesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            { indices.push_back(face.mIndices[j]); }
        }

        modelMesh.mesh.setIndices(indices);

        // process material
        if (aimesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];
            loadMaterialTextures(modelMesh.textures, material, aiTextureType_DIFFUSE, path);
            //loadMaterialTextures(modelMesh.textures, material, aiTextureType_SPECULAR, path);
        }
    }

    void processNode(aiNode* node, const aiScene* scene, const fs::path& path)
    {
        static size_t processed = 0;
        tlog::info("Processing node: {}", processed);
        ++processed;

        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            tlog::info("Processing mesh: {}", i);
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh,scene,path);
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i],scene,path);
        }
    }

public:
    Vector<ModelMesh>& getMeshes()
    { return meshes; }

    // Returns false on failure
    bool loadFromFile(const fs::path& path)
    {
        // https://learnopengl.com/Model-Loading/Model

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(),
            aiProcess_Triangulate           |
            aiProcess_GenNormals            |
            aiProcess_OptimizeMeshes        |
            aiProcess_OptimizeGraph         |
            aiProcess_JoinIdenticalVertices | // TODO: Read somewhere this flag could cause problems, maybe look into it.
            aiProcess_FlipUVs); // Renderer class already handles the OpenGL conversion.

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            tlog::error("ASSIMP: Failed to load mesh\nReason: {}\nPath: {}\nWorking Directory: {}",
                importer.GetErrorString(), path.string(), fs::current_path().string());
            return false;
        }

        reset();
        processNode(scene->mRootNode, scene, path.parent_path());

        return true;
    }

    void reset()
    {
        meshes.clear();
    }
};

struct Transform
{
    Vector3f  pos;
    Vector3f  scale = { 1, 1, 1 };
    glm::quat rot{glm::vec3(0.f, 0.f, 0.f)};

    glm::mat4 getMatrix() const
    {
        // Scale, rotate, translate
        glm::mat4 mat(1.f);
        mat  = glm::scale(mat, { scale.x, scale.y, scale.z });
        mat *= glm::toMat4(rot);
        mat  = glm::translate(mat, { pos.x, pos.y, pos.z });
        return mat;
    }

    void rotate(float angle, const Vector3f& axis)
    {
        rot *= glm::angleAxis(angle, glm::vec3{axis.x, axis.y, axis.z});
    }
};

class Renderer3D
{
    struct DrawCmd
    {
        Model&    model;
        Transform transform;

        DrawCmd(Model& m, const Transform& tf) :
            model{m}, transform{tf} { }
    };

    static inline Vector<DrawCmd> cmds;

    static inline Shader  shader3d;
    static inline Texture defaultTex;

    static inline GLubyte defaultTexColor[4] = {255, 255, 255, 255};

    static constexpr int defaultTexDataSize = 32;
    static inline GLubyte defaultTexData[defaultTexDataSize][defaultTexDataSize][4];

    static inline void setupRender()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
    }

    static inline void drawMeshImmediate(Mesh& mesh, Texture& tex, const Transform& transform)
    {
        tex.bind();
        shader3d.setMat4f("model", transform.getMatrix());
        Renderer::draw(shader3d, mesh);
    }

public:
    static inline void render()
    {
        setupRender();

        for (auto& cmd : cmds)
        {
            for (auto& mesh : cmd.model.getMeshes())
            {
                // If there's no texture, use default texture
                if (mesh.textures.empty())
                {
                    drawMeshImmediate(mesh.mesh, defaultTex, cmd.transform);
                }
                else
                {
                    for (auto& tex : mesh.textures)
                    {
                        if (tex.type == Model::TextureType::aiTextureType_DIFFUSE)
                        {
                            // Only use diffuse for now
                            // TODO: use the rest of the textures
                            // make a uniform block for all of them, and set them all at once.
                            ASSERT(tex.texture && tex.texture->valid());
                            drawMeshImmediate(mesh.mesh, *tex.texture, cmd.transform);
                            break;
                        }
                    }
                }
            }
        }

        cmds.clear();
    }

    static inline void create()
    {
        shader3d.create(vert_3d, frag_3d);
        defaultTex.create();

        // Make the default texture look more interesting
        RNG rng;
        for (size_t x = 0; x < defaultTexDataSize; x++)
        {
            for (size_t y = 0; y < defaultTexDataSize; y++)
            {
                const int diff = 20;
                GLubyte color[3] = {70, 130, 180};
                color[0] = std::clamp( rng.randRangeInt(color[0] - diff, color[0] + diff), 0, 255);
                color[1] = std::clamp( rng.randRangeInt(color[1] - diff, color[1] + diff), 0, 255);
                color[2] = std::clamp( rng.randRangeInt(color[2] - diff, color[2] + diff), 0, 255);

                defaultTexData[x][y][0] = color[0];
                defaultTexData[x][y][1] = color[1];
                defaultTexData[x][y][2] = color[2];
                defaultTexData[x][y][3] = 255;
            }
        }
        defaultTex.setData(&defaultTexData, defaultTexDataSize, defaultTexDataSize);
        defaultTex.setUVMode(UVMode::Repeat);
        defaultTex.setFilter(TextureFiltering::Linear);
    }

    static inline bool created()
    {
        return shader3d.created();
    }

    static inline void setCamera(const Camera3D& camera)
    {
        auto view = camera.getViewMatrix();
        auto proj = camera.getPerspectiveMatrix();
        shader3d.setMat4f("projection", proj);
        shader3d.setMat4f("view", view);
    }

    static inline void drawModel(Model& model, const Transform& transform)
    {
        DrawCmd& cmd = cmds.emplace_back(model, transform);
    }

    static inline void addLight(const Vector3f& pos, const ColorRGBf& color)
    {
        shader3d.setVec3f("lightPos", pos.x, pos.y, pos.z);
        shader3d.setVec3f("lightColor", color.r, color.g, color.b);
    }
};

enum class CameraMode
{
    Free,
    Orbit
};

Window     window;
MyGui      imgui;
FPSLimit   fpslimit;
Timer      deltaTimer;
Camera3D   camera;
CameraMode cameraMode = CameraMode::Orbit;

// Cam vars
float sens = 0.1f;
float camSpeed = 3.f;
float yaw = 0.f;
float pitch = 0.f;

float    orbitDist = 5.f;
Vector2f orbitAngle;

void setCameraMode(CameraMode mode)
{
    switch (mode)
    {
    case CameraMode::Free:
        window.setFpsMode(true);
        break;
    case CameraMode::Orbit:
        window.setFpsMode(false);
        break;
    default: break;
    }
    cameraMode = mode;
}

void resetCamera()
{
    camera     = Camera3D();
    pitch      = 0;
    yaw        = 0;
    orbitDist  = 5;
    orbitAngle = {};
}

int main()
{
    WindowCreateParams p;
    p.size  = { 1280, 720 };
    p.title = "Model";
    window.create(p);
    Renderer::create();
    Renderer2D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);
    
    Model      model;
    //Model    lightModel;
    Transform  modelTransform;

    const char* presetModelPaths[7] =
    {
        "assets/sponza/sponza.obj",
        "assets/backpack/backpack.obj",
        "assets/primitives/cube.obj",
        "assets/primitives/cylinder.obj",
        "assets/primitives/monkey.obj",
        "assets/primitives/sphere.obj",
        "assets/primitives/torus.obj"
    };
    int guiSelectedModel = 0;
    String guiModel;
    float guiLightColor[3] = { 0.5f, 0.5f, 0.5f };

    Renderer3D::create();
    RELASSERTMSGBOX(model.loadFromFile(presetModelPaths[0]),
        "Model not found",
        fmt::format("Failed to find init model: {}", presetModelPaths[0]).c_str());

    //lightModel.loadFromFile("assets/primitives/sphere.obj");

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = Renderer2D::getView();
                view.size = Vector2f(e.window.data1, e.window.data2);
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse))    { Input::updateMouse(); }

        ////
        imgui.newFrame();
        ImGui::BeginDisabled(window.getFpsMode());
        beginDiagWidgetExt();

        Vector2f mwpos = Renderer2D::getMouseWorldPos();

        glClear(GL_DEPTH_BUFFER_BIT);
        Renderer::clearColor();

        static float time = 0.f;
        time += delta;

        auto result = imguiEnumCombo("Camera Mode", cameraMode);
        if (result.first)
        { setCameraMode(result.second); }
        if (ImGui::Button("Reset Camera"))
        { resetCamera(); }

        switch (cameraMode)
        {
        case CameraMode::Free:
        {
            ImGui::Text("Press ALT to toggle cursor.");
            ImGui::SliderFloat("Sensitivity", &sens, 0.01f, 2.f);
            ImGui::SliderFloat("Camera Speed", &camSpeed, 0.01f, 200.f);
            if (Input::isKeyJustPressed(SDL_SCANCODE_LALT))
            { window.toggleFpsMode(); }

            if (window.getFpsMode())
            {
                yaw += float(Input::mouseDelta.x) * sens;
                pitch -= float(Input::mouseDelta.y) * sens;
                pitch = std::clamp(pitch, -89.f, 89.f);
            }

            glm::vec3 direction{};
            direction.x      = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y      = sin(glm::radians(pitch));
            direction.z      = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            auto cameraFront = glm::normalize(direction);

            Vector2f moveDir;
            moveDir.x = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
            moveDir.y = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);
            if (moveDir.x)
            { camera.pos += glm::normalize(glm::cross(cameraFront, camera.up)) * camSpeed * moveDir.x * delta; }
            if (moveDir.y)
            { camera.pos += camSpeed * cameraFront * moveDir.y * delta; }

            camera.target = camera.pos + glm::normalize(direction);

            break;
        }
        case CameraMode::Orbit:
        {
            // Zoom
            float scrollDelta = Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP) - Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN);
            if (scrollDelta != 0.f)
            { orbitDist = std::clamp(orbitDist - (scrollDelta * 0.5f), 1.f, 20.f); }

            if (Input::isMousePressed(Input::MOUSE_MIDDLE))
            {
                auto mdelta = Input::mouseDelta;
                orbitAngle.x += mdelta.x;
                orbitAngle.y -= mdelta.y;
                orbitAngle.y = std::clamp(orbitAngle.y, -89.f, 89.f);
            }

            // https://gamedev.stackexchange.com/questions/20758/how-can-i-orbit-a-camera-about-its-target-point
            float camX = orbitDist * -sinf(orbitAngle.x * (M_PI / 180)) * cosf((orbitAngle.y) * (M_PI / 180));
            float camY = orbitDist * -sinf((orbitAngle.y) * (M_PI / 180));
            float camZ = orbitDist * cosf((orbitAngle.x) * (M_PI / 180)) * cosf((orbitAngle.y) * (M_PI / 180));

            camera.setPos({ camX, camY, camZ });
            
            break;
        }
        default: break;
        }

        Renderer3D::setCamera(camera);

        //float moveInputX    = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
        //float moveInputY    = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);
        //float moveInputRoll = Input::isKeyPressed(SDL_SCANCODE_E) - Input::isKeyPressed(SDL_SCANCODE_Q);
        //modelTransform.rotate(moveInputX * delta, { 0.f, 1.f, 0.f });
        //modelTransform.rotate(moveInputY * delta, { 1.f, 0.f, 0.f });
        //modelTransform.rotate(moveInputRoll * delta, { 0.f, 0.f, 1.f });

        const float radius = 20.0f;
        float lightX = sin(time) * radius;
        float lightZ = cos(time) * radius;
        Vector3f lightPos = { lightX, 0.f, lightZ };
        Renderer3D::addLight(lightPos, { guiLightColor[0], guiLightColor[1], guiLightColor[2] });

        //Transform lightModelTf;
        //lightModelTf.pos = lightPos;
        //Renderer3D::drawModel(lightModel, lightModelTf);

        Renderer3D::drawModel(model, modelTransform);
        Renderer2D::drawCircle(mwpos, 12.f);

        // Model loading input
        if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
        { model.loadFromFile(presetModelPaths[guiSelectedModel]); }
        ImGui::InputText("Model Path", &guiModel);
        if (ImGui::Button("Load"))
        { model.loadFromFile(guiModel); }
        ImGui::ColorEdit3("Lighting", guiLightColor);
        cameraMode = imguiEnumCombo("Camera Mode", cameraMode).second;
        ImGui::End();
        ////

        Renderer3D::render();
        Renderer2D::render();
        drawDiagWidget(&fpslimit);
        ImGui::EndDisabled();
        imgui.render();
        window.swap();
        fpslimit.wait();
    }

    return 0;
}