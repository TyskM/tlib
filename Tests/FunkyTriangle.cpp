//
// Created by Ty on 2023-01-27.
//

#include "Common.hpp"

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
        mesh.setData(triVerts, AccessType::Dynamic);
        mesh.setLayout({ Layout::Vec2f(), Layout::Vec4f() });
        shader.create(vert_flat, frag_flat);
        Camera2D view;
        view.setBounds({ 0, 0, 1, 1 });
        shader.setMat4f("projection", view.getMatrix());
    }

    Vector2f pos;

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);

        if (Input::isKeyJustPressed(SDL_SCANCODE_W))
        { pos.y += 10.f * 1/60.f; }
        if (Input::isKeyJustPressed(SDL_SCANCODE_S))
        { pos.y -= 10.f * 1/60.f; }

        Camera2D view;
        view.setBounds({ pos.x, pos.y, 1, 1 });
        shader.setMat4f("projection", view.getMatrix());

        renderer.clearColor();
        makeTriangleFunky();
        renderer.draw(shader, mesh);
        window.swap();
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
    TriangleTest game;
    game.create();
    game.run();
    return 0;
}