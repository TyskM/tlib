
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
#include <TLib/Embed/Embed.hpp>
#include <TLib/Containers/Stack.hpp>

struct MeshSlice
{
    size_t vboIndex = 0;
    size_t vboSize  = 0;
    size_t eboIndex = 0;
    size_t eboSize  = 0;
};

enum class ViewMode
{
    Perspective,
    Orthographic
};

struct Camera3D
{
    float     znear    = 0.02f;
    float     zfar     = 3000.f;
    float     fov      = 110.f;
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

        Path fileFolder = Path(path).remove_filename();

        static Stack<aiNode*>   stack;
        static Vector<Vertex>   vertices;
        static Vector<uint32_t> indices;
        stack.get_container().clear();
        stack.push(scene->mRootNode);
        while (!stack.empty())
        {
            aiNode* node = stack.top();
            stack.pop();

            // process all the node's meshes (if any)
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

                ModelMesh& modelMesh = meshes.emplace_back();
                modelMesh.mesh.setLayout({ Layout::Vec3f(), Layout::Vec3f(), Layout::Vec2f() });

                // process vertex positions, normals and texture coordinates
                vertices.clear();
                vertices.reserve(mesh->mNumVertices);
                for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                {
                    Vertex vertex;

                    auto& pos = mesh->mVertices[i];
                    vertex.position = glm::vec3(pos.x, pos.y, pos.z);

                    auto& normal = mesh->mNormals[i];
                    vertex.normal = glm::vec3(normal.x, normal.y, normal.z);

                    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                    { vertex.texCoords ={ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }; }
                    else
                    { vertex.texCoords = glm::vec2(0.0f, 0.0f); }

                    vertices.push_back(vertex);
                }

                modelMesh.mesh.setData(vertices);

                // process indices
                indices.clear();
                for (unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    aiFace face = mesh->mFaces[i];
                    for (unsigned int j = 0; j < face.mNumIndices; j++)
                    { indices.push_back(face.mIndices[j]); }
                }

                modelMesh.mesh.setIndices(indices);
                // process material
                if (mesh->mMaterialIndex >= 0)
                {
                    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                    loadMaterialTextures(modelMesh.textures, material, aiTextureType_DIFFUSE, fileFolder);
                    //loadMaterialTextures(modelMesh.textures, material, aiTextureType_SPECULAR, path);
                }
            }

            // Queue children for above process
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                stack.push(node->mChildren[i]);
            }
        }

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

struct MeshData
{
    using TextureType = aiTextureType;

    struct MeshTexture
    {
        TextureType type    = aiTextureType_NONE;
        Texture*    texture = nullptr;
    };

    struct SubMesh
    {

    };

    static Vector<Vertex>   vertices;
    static Vector<uint32_t> indices;
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

public:
    static inline Vector<DrawCmd> cmds;

    static inline Shader  shader3d;

    static inline Texture emptyTex;
    static inline GLubyte emptyTexData[1][1][4] = {{{255,255,255,255}}};

    static inline Texture defaultTex;
    static constexpr int  defaultTexDataSize = 32;
    static inline GLubyte defaultTexColor[4] = {255, 255, 255, 255};
    static inline GLubyte defaultTexData[defaultTexDataSize][defaultTexDataSize][4];

    static inline bool useMSAA      = false; // Read / Write. Creates ugly artifacts, find post processing solution.
    static inline bool useDithering = false; // Read / Write. Is a no op on most implementations.

private:

    static inline FrameBuffer shadowFbo;
    static inline Texture     shadowTex;
    static inline uint32_t    shadowSize = 1024;
    static inline Shader      shadowShader;

    static void drawLightDepthMap()
    {
        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView =
            glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                        glm::vec3( 0.0f, 0.0f,  0.0f),
                        glm::vec3( 0.0f, 1.0f,  0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    }

    static void setupRender()
    {
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glFrontFace(GL_CCW));
        GL_CHECK(glEnable(GL_DITHER));

        if (useDithering) { GL_CHECK( glEnable(GL_DITHER)); }
        else              { GL_CHECK(glDisable(GL_DITHER)); }

        if (useMSAA) { GL_CHECK( glEnable(GL_MULTISAMPLE)); }
        else         { GL_CHECK(glDisable(GL_MULTISAMPLE)); }

        shader3d.setInt  ("material.diffuse",    0);
        shader3d.setInt  ("material.specular",   1);
        shader3d.setFloat("material.shininess", 32);
    }

    static void drawMeshImmediate(Mesh& mesh, Texture& tex, const Transform& transform)
    {
        tex.bind();
        emptyTex.bind(1);
        shader3d.setMat4f("model", transform.getMatrix());
        Renderer::draw(shader3d, mesh);
    }

public:
    static void render()
    {
        setupRender();

        Renderer::setViewport(Recti(Vector2i(0), Vector2i(shadowSize, shadowSize)));
        shadowFbo.bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            // Shadows here
        shadowFbo.unbind();
        Renderer2D::setView(Renderer2D::getView());

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

    static void create()
    {
        defaultTex.create();

        shader3d.create(myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString(),
                        myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString());

        emptyTex.create();
        emptyTex.setData(emptyTexData, 1, 1);

        // Setup shadows
        shadowTex.create();
        shadowTex.setData(NULL, shadowSize, shadowSize, TexPixelFormats::DEPTH_COMPONENT, TexInternalFormats::DEPTH_COMPONENT);
        shadowTex.setFilter(TextureFiltering::Nearest);
        shadowTex.setUVMode(UVMode::Repeat);
        shadowFbo.create();
        shadowFbo.setTexture(shadowTex, FrameBufferAttachmentType::Depth);
        shadowShader.create(myEmbeds.at("TLib/Embed/Shaders/light.vert").asString(),
                            myEmbeds.at("TLib/Embed/Shaders/empty.frag").asString());

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

    static bool created()
    {
        return shader3d.created();
    }

    static void setCamera(const Camera3D& camera)
    {
        auto view = camera.getViewMatrix();
        auto proj = camera.getPerspectiveMatrix();
        shader3d.setMat4f("projection", proj);
        shader3d.setMat4f("view", view);
        shader3d.setVec3f("viewPos", camera.pos);
    }

    static void drawModel(Model& model, const Transform& transform)
    {
        DrawCmd& cmd = cmds.emplace_back(model, transform);
    }

    static void addLight(const Vector3f& pos, const ColorRGBf& color)
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

String vertShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString();
String fragShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString();

// Cam vars
float sens     = 0.1f;
float camSpeed = 12.f;
float yaw      = 0.f;
float pitch    = 0.f;

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
    using R3D = Renderer3D;
    R3D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);
    
    Model      model;
    Transform  modelTransform;

    const char* presetModelPaths[7] =
    {
        "assets/primitives/cube.obj",
        "assets/sponza/sponza.obj",
        "assets/backpack/backpack.obj",
        "assets/primitives/cylinder.obj",
        "assets/primitives/monkey.obj",
        "assets/primitives/sphere.obj",
        "assets/primitives/torus.obj"
    };
    int guiSelectedModel = 0;
    String guiModel;
    float guiLightColor[3] = { 0.5f, 0.5f, 0.5f };

    
    RELASSERTMSGBOX(model.loadFromFile(presetModelPaths[0]),
        "Model not found",
        fmt::format("Failed to find init model: {}", presetModelPaths[0]).c_str());

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

        ImGui::SeparatorText("Camera");
        auto result = imguiEnumCombo("Camera Mode", cameraMode);
        if (result.first)
        { setCameraMode(result.second); }
        if (ImGui::Button("Reset Camera"))
        { resetCamera(); }
        ImGui::SliderFloat("FOV", &camera.fov, 70.f, 170.f);

        switch (cameraMode)
        {
        case CameraMode::Free:
        {
            ImGui::Text("Press ALT to toggle cursor.");
            ImGui::SliderFloat("Sensitivity", &sens, 0.01f, 2.f);
            ImGui::SliderFloat("Camera Speed", &camSpeed, 0.01f, 400.f);
            if (Input::isKeyJustPressed(SDL_SCANCODE_LALT))
            { window.toggleFpsMode(); }

            if (window.getFpsMode())
            {
                yaw   += float(Input::mouseDelta.x) * sens;
                pitch -= float(Input::mouseDelta.y) * sens;
                pitch  = std::clamp(pitch, -89.f, 89.f);
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

        R3D::setCamera(camera);

        const float radius = 20.0f;
        float lightX = sin(time) * radius;
        float lightZ = cos(time) * radius;
        Vector3f lightPos = { lightX, 0.f, lightZ };
        R3D::addLight(lightPos, { guiLightColor[0], guiLightColor[1], guiLightColor[2] });

        R3D::drawModel(model, modelTransform);
        //Renderer2D::drawCircle(mwpos, 12.f);

        // Model loading input
        ImGui::SeparatorText("Model");
        if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
        { model.loadFromFile(presetModelPaths[guiSelectedModel]); }
        if (ImGui::Button("Load Custom Model"))
        {
            Path p = openSingleFileDialog();
            if (!p.empty()) { model.loadFromFile(p); }
        }
        
        ImGui::SeparatorText("Shaders");

        ImGui::ColorEdit3("Lighting", guiLightColor);

        ImGui::Checkbox("MSAA", &R3D::useMSAA);
        ImGui::Checkbox("Dithering", &R3D::useDithering);

        static float ambientStrength = R3D::shader3d.getFloat("ambientStrength");
        if (ImGui::SliderFloat("Ambient Lighting", &ambientStrength, 0.f, 1.f))
        { R3D::shader3d.setFloat("ambientStrength", ambientStrength); }

        static float specularStrength = R3D::shader3d.getFloat("specularStrength");
        if (ImGui::SliderFloat("Specular Strength", &specularStrength, 0.f, 1.f))
        { R3D::shader3d.setFloat("specularStrength", specularStrength); }

        static glm::vec3 sunDir = R3D::shader3d.getVec3f("sunDir");
        if (ImGui::DragFloat3("Sun Dir", &sunDir.x, 0.025f, -1.f, 1.f))
        { R3D::shader3d.setVec3f("sunDir", sunDir); }

        if (ImGui::Button("Compile Shaders"))
        { R3D::shader3d.create(vertShaderStr, fragShaderStr); }
        if (ImGui::CollapsingHeader("Vert Shader"))
        { ImGui::InputTextMultiline("#VertShaderEdit", &vertShaderStr, {ImGui::GetContentRegionAvail().x, 600}); }
        if (ImGui::CollapsingHeader("Frag Shader"))
        { ImGui::InputTextMultiline("#FragShaderEdit", &fragShaderStr, { ImGui::GetContentRegionAvail().x, 600 }); }

        ImGui::SeparatorText("Diag");
        ImGui::End();
        ////

        R3D::render();
        Renderer2D::render();
        drawDiagWidget(&fpslimit);
        ImGui::EndDisabled();
        imgui.render();
        window.swap();
        fpslimit.wait();
    }

    return 0;
}