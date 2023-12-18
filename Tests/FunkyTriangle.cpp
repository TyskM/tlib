//
// Created by Ty on 2023-01-27.
//

#include "Common.hpp"

const char* frag_flat_floored = R"""(
        #version 330 core
        out vec4 FragColor;
        in vec4 color;

        uniform int res = 24;

        void main()
        {
            vec4 col = floor(color * res) / res;
            FragColor = col;
        }
        )""";

struct TriangleTest : GameTest
{
    Mesh mesh;
    Shader shader;

    struct Vert
    {
        Vector2f pos;
        ColorRGBAf color;
    };

    std::vector<Vert> triVerts =
    {
        { { 0.5f, 0.2f }, ColorRGBAf::red() },
        { { 0.2f, 0.8f }, ColorRGBAf::red() },
        { { 0.8f, 0.8f }, ColorRGBAf::red() }
    };

    void create() override
    {
        GameTest::create();
        window.setTitle("Funky Triangle");
        mesh.setLayout({ Layout::Vec2f(), Layout::Vec4f() });
        mesh.setData(triVerts, AccessType::Dynamic);
        shader.create(vert_flat, frag_flat_floored);
        View view;
        view.setBounds({ 0, 0, 1, 1 });
        shader.setMat4f("projection", view.getMatrix());
    }

    Vector2f pos;

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);

        const float ms = 2.f;
        if (Input::isKeyPressed(SDL_SCANCODE_W))
        { pos.y += ms * delta; }
        if (Input::isKeyPressed(SDL_SCANCODE_S))
        { pos.y -= ms * delta; }
        if (Input::isKeyPressed(SDL_SCANCODE_A))
        { pos.x += ms * delta; }
        if (Input::isKeyPressed(SDL_SCANCODE_D))
        { pos.x -= ms * delta; }

        if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP))
        { shader.setInt("res", shader.getInt("res") + 1); }
        else if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN))
        { shader.setInt("res", shader.getInt("res") - 1); }

        View view;
        view.setBounds({ pos.x, pos.y, 1, 1 });
        shader.setMat4f("projection", view.getMatrix());

        Renderer::clearColor();
        makeTriangleFunky();
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
            v.color.r = 0.6f + std::fmod(std::sin(i * time    ), 1.f);
            v.color.g = 0.3f + std::fmod(std::sin(i * time * 2), 1.f);
            v.color.b = 0.1f + std::fmod(std::sin(i * time * 3), 1.f);
            ++i;
        }
        mesh.setData(triVerts, AccessType::Dynamic);
    }
};

int main()
{
    TriangleTest game;
    game.create();
    game.run();
    return 0;
}