
#include <TLib/Media/Resource/GPUVertexData.hpp>
#include <TLib/Media/GL/Shader.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Types/Types.hpp>
#include <TLib/Timer.hpp>

const char* vert = R"""(
#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 textureUV;
layout (location = 2) in mat4 transform;

out vec2 fragTextureUV;
out vec4 fragColor;

uniform mat4 projection;

void main()
{
    gl_Position   = projection * transform * vec4(vertex, 0.0, 1.0);
    fragTextureUV = textureUV;
    fragColor     = vec4(1,1,1,1);
}
)""";

const char* frag = R"""(
#version 330 core
in  vec2 fragTextureUV;
in  vec4 fragColor;
out vec4 outColor;

uniform sampler2D image;

void main()
{
    outColor = fragColor * texture(image, fragTextureUV);
}
)""";

struct QuadVertex
{
    Vector2f position;
    Vector2f uv;
};

ColorRGBAf         clearColor = ColorRGBAf::black();
GPUVertexData      mesh;
Shader             shader;
View               view;
Texture            texture;

Array<uint32_t,   6> indices  = { 0, 1, 2, 0, 2, 3 };
Array<QuadVertex, 4> vertices =
{
    QuadVertex{ Vector2f(-0.5f,  0.5f), Vector2f(0.0f, 1.0f) },
    QuadVertex{ Vector2f( 0.5f,  0.5f), Vector2f(1.0f, 1.0f) },
    QuadVertex{ Vector2f( 0.5f, -0.5f), Vector2f(1.0f, 0.0f) },
    QuadVertex{ Vector2f(-0.5f, -0.5f), Vector2f(0.0f, 0.0f) }
};

Vector<Mat4f> quads;

static void init()
{
    shader.create(vert, frag);

    texture.loadFromFile("assets/ship.png");

    mesh.setLayout({ TLib::Layout::Vec2f(), TLib::Layout::Vec2f() }, 0, 0);
    mesh.setData(vertices, AccessType::Static, 0);
    mesh.setIndices(indices);
}

static void shutdown()
{

}

static void update(float delta)
{

}

static void draw(float delta)
{
    Vector2f mouseLocalPos = Vector2f(Input::mousePos);
    Vector2f mouseWorldPos = localToWorldPoint(mouseLocalPos, view, Renderer::getFramebufferSize());

    clearColor.r = mouseWorldPos.x / 1280.f;
    clearColor.b = mouseWorldPos.y / 720.f;

    shader.setMat4f("projection", view.getMatrix());

    Renderer::clearColor(clearColor);

    RenderState rs;
    rs.mesh     =   &mesh;
    rs.shader   =   &shader;
    rs.textures = { &texture };

    quads.clear();
    auto& tf = quads.emplace_back();
    tf = Mat4f(1).translated(mouseWorldPos).scaled(Vector2f(200.f));
    TLib::Layout layout(TLib::Layout::Mat4f(), 1);
    layout.setDivisor(1);
    mesh.setLayout(layout, 1, 2);
    mesh.setData(quads, AccessType::Dynamic, 1);
    Renderer::drawElementsInstanced(rs, quads.size());
}

int main()
{
    Window   window;
    FPSLimit fpslimit;
    MyGui    imgui;
    Timer    deltaTimer;

    WindowCreateParams params;
    params.title = "Window";
    params.size  = {1280, 720};
    window.create(params);
    Input::init(window);
    Renderer::create();

    imgui       .create(window);
    deltaTimer  .restart();
    fpslimit    .setFPSLimit(144);
    fpslimit    .setEnabled(true);

    init();

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
                view.size = Vector2f(e.window.data1, e.window.data2);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        imgui.newFrame();
        update(delta);
        draw(delta);

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    shutdown();

    return 0;
}
