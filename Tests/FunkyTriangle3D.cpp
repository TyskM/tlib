//
// Created by Ty on 2023-01-27.
//

#include "Common.hpp"

enum class ViewMode
{
    Perspective,
    Orthographic
};

struct Camera3D
{
protected:
    float znear  = 0.05f;
    float zfar   = 30.f;
    float fov    = 90.f;
    glm::vec3 pos    = { 0.0f, 0.0f, 0.0f };
    glm::vec3 target = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up     = { 0.0f, 1.0f, 0.0f };
    ViewMode viewmode = ViewMode::Perspective;

public:
    inline void setPos(const Vector3f& posv) { this->pos = {posv.x, posv.y, posv.z}; }
    [[nodiscard]] inline Vector3f getPos() const { return Vector3f(pos); }

    inline void setTarget(const Vector3f& targetv) { target = { targetv.x, targetv.y, targetv.z }; }
    [[nodiscard]] inline Vector3f getTarget() const { return Vector3f(target); }

    inline void setUp(const Vector3f& upv) { up = { upv.x, upv.y, upv.z }; }
    [[nodiscard]] inline Vector3f getUp() const { return Vector3f(up); }

    [[nodiscard]]
    glm::mat4 getViewMatrix() const
    {
        //glm::vec3 cameraDirection = glm::normalize(pos - target);
        //glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        //glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
        //return glm::lookAt(pos, target, cameraUp);
        return glm::lookAt(pos, target, up);
    }

    [[nodiscard]]
    glm::mat4 getPerspectiveMatrix()
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

struct TriangleTest3D : GameTest
{
    Mesh mesh;
    Shader shader;
    Camera3D camera;

    struct Vert
    {
        Vector3f pos;
        ColorRGBAf color;
    };

    std::vector<Vert> triVerts =
            {
                { {  0.0f,  0.4f, 0.f }, ColorRGBAf::red() }, // top
                { { -0.3f, -0.2f, 0.f }, ColorRGBAf::red() }, // bl
                { {  0.3f, -0.2f, 0.f }, ColorRGBAf::red() }  // br
            };

    void create() override
    {
        GameTest::create();
        window.setTitle("Funky Triangle 3D");
        glDisable(GL_CULL_FACE);
        mesh.setLayout({ Layout::Vec3f(), Layout::Vec4f() });
        mesh.setData(triVerts, AccessType::Dynamic);
        shader.create(vert_flat3d, frag_flat);
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);

        Renderer::clearColor();
        makeTriangleFunky();

        const float radius = 1.0f;
        float x = std::sin(timer.getElapsedTime().asSeconds()) * radius;
        float z = std::cos(timer.getElapsedTime().asSeconds()) * radius;

        auto pos = camera.getPos();
        camera.setPos({ x, pos.y, z });
        auto view = camera.getViewMatrix();
        auto proj = camera.getPerspectiveMatrix();
        shader.setMat4f("projection", proj * view);

        Renderer::draw(shader, mesh);
        window.swap();
        fpslimit.wait();
    }

    void makeTriangleFunky()
    {
        float time = timer.getElapsedTime().asSeconds();
        int i = 0;
        for (auto& v : triVerts)
        {
            v.color.r = std::fmod(std::sin(i * time    ), 1.f);
            v.color.g = std::fmod(std::sin(i * time * 2), 1.f);
            v.color.b = std::fmod(std::sin(i * time * 3), 1.f);
            ++i;
        }
        mesh.setData(triVerts, AccessType::Dynamic);
    }
};

int main()
{
    TriangleTest3D game;
    game.create();
    game.run();
    return 0;
}