//
// Created by Ty on 2023-01-27.
//

#include "Common.hpp"

struct IndicesTest : GameTest
{
    Mesh mesh;
    Shader shader;

    struct Vert
    {
        Vector2f pos;
        ColorRGBAf color;
    };

    std::vector<Vert> vertices =
    {
        Vert{ { 0.1f, 0.75f }, ColorRGBAf::blue() },
        Vert{ { 0.3f, 0.5f  }, ColorRGBAf::blue() },
        Vert{ { 0.5f, 0.75f }, ColorRGBAf::blue() },
        Vert{ { 0.7f, 0.5f  }, ColorRGBAf::blue() },
        Vert{ { 0.9f, 0.75f }, ColorRGBAf::blue() },
        Vert{ { 0.5f, 0.25f }, ColorRGBAf::blue() }
    };

    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 4, 1, 5, 3 };

    void create() override
    {
        GameTest::create();
        mesh.setData(vertices);
        mesh.setIndices(indices);
        mesh.setLayout({ Layout::Vec2f(), Layout::Vec4f() });
        shader.create(vert_flat, frag_flat);
        Camera2D view;
        view.setBounds({ 0, 0, 1, 1 });
        shader.setMat4f("projection", view.getMatrix());
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);

        renderer.clearColor();
        renderer.draw(shader, mesh);
        window.swap();
    }
};

int main()
{
    IndicesTest game;
    game.create();
    game.run();
    return 0;
}