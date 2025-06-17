
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
#include <TLib/Embed/Embed.hpp>

ColorRGBAf         clearColor = ColorRGBAf::black();
GPUVertexData      mesh;
Shader             shader;
View               view;
Texture            texture;

// Creates a mesh for use with drawElementsInstanced
struct SpriteBatcher
{
private:
    struct QuadVertex
    {
        Vector2f position;
        Vector2f uv;
    };

    struct QuadVertexInstance
    {
        ColorRGBAf color = ColorRGBAf::white();
        Mat4f      transform;
    };

    static constexpr Array<uint32_t,   6> indices  = { 0, 1, 2, 0, 2, 3 };
    static constexpr Array<QuadVertex, 4> vertices =
    {
        QuadVertex{ Vector2f(-0.5f,  0.5f), Vector2f(0.0f, 1.0f) },
        QuadVertex{ Vector2f( 0.5f,  0.5f), Vector2f(1.0f, 1.0f) },
        QuadVertex{ Vector2f( 0.5f, -0.5f), Vector2f(1.0f, 0.0f) },
        QuadVertex{ Vector2f(-0.5f, -0.5f), Vector2f(0.0f, 0.0f) }
    };

    Vector<QuadVertexInstance> instances;

public:
    size_t size() const { return instances.size(); }

    void append(const Mat4f& transform, const ColorRGBAf& color = ColorRGBAf::white())
    {
        auto& instance     = instances.emplace_back();
        instance.transform = transform;
        instance.color     = color;
    }

    void append(const Vector2f&   position,
                const float       rotation,
                const Vector2f&   size  = Vector2f(100.f, 100.f),
                const ColorRGBAf& color = ColorRGBAf::white())
    {
        Mat4f transform = Mat4f(1).translated(position).rotatedZ(rotation).scaled(size);
        append(transform, color);
    }

    void clear()
    {
        instances.clear();
    }

    void upload(GPUVertexData& mesh)
    {
        // Set vertex data
        mesh.setLayout({ TLib::Layout::Vec2f(), TLib::Layout::Vec2f() }, 0, 0);
        mesh.setData(vertices, AccessType::Static, 0);
        mesh.setIndices(indices);

        // Set instance data
        TLib::Layout layout({ TLib::Layout::Vec4f(), TLib::Layout::Mat4f() });
        layout.setDivisor(1);
        mesh.setLayout(layout, 1, 2);
        mesh.setData(instances, AccessType::Dynamic, 1);
    }
};

float t = 0.f;

static void init()
{
    shader.create(myEmbeds["TLib/Embed/Shaders/2d.vert"].asString(),
                  myEmbeds["TLib/Embed/Shaders/2d.frag"].asString());

    texture.loadFromFile("assets/ship.png");
}

static void shutdown()
{

}

static void update(float delta)
{
    t += delta;
}

static void draw(float delta)
{
    Vector2f mouseLocalPos = Vector2f(Input::mousePos);
    Vector2f mouseWorldPos = localToWorldPoint(mouseLocalPos, view, Renderer::getFramebufferSize());

    clearColor.r = mouseWorldPos.x / 1280.f;
    clearColor.b = mouseWorldPos.y / 720.f;

    shader.setMat4f("projection", view.getMatrix());

    Renderer::clearColor(clearColor);

    SpriteBatcher bs;
    bs.append(mouseWorldPos,          0.f, 200.f, ColorRGBAf(sin(t), 0.f, 0.f, 1.f));
    bs.append(mouseWorldPos + 150.f,  3.f, 150.f, ColorRGBAf(0.f, sin(t), 0.f, 1.f));
    bs.append(mouseWorldPos - 150.f, -3.f, 250.f, ColorRGBAf(0.f, 0.f, sin(t), 1.f));
    bs.upload(mesh);

    RenderState rs;
    rs.mesh     =   &mesh;
    rs.shader   =   &shader;
    rs.textures = { &texture };

    Renderer::drawElementsInstanced(rs, bs.size());
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
