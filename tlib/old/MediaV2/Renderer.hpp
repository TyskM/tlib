#pragma once

#include "../DataStructures.hpp"
#include "View.hpp"
#include "Window.hpp"
#include "Texture.hpp"

#include <Magnum/Primitives/Square.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/MeshData2D.h>

struct Renderer
{
    using Mat3         = Magnum::Matrix3;
    using MVec2        = Magnum::Vector2;
    using MVec3        = Magnum::Vector3;
    using MColor4      = Magnum::Color4;
    using Rad          = Magnum::Rad;
    using Deg          = Magnum::Deg;
    using Mesh         = Magnum::GL::Mesh;
    using ShaderFlat2D = Magnum::Shaders::Flat2D;
    using Buffer       = Magnum::GL::Buffer;

    Window* window;
    typedef Magnum::GL::Attribute<0, Magnum::Vector2> Position;
    typedef Magnum::GL::Attribute<1, Magnum::Vector2> TextureCoordinates;

    struct QuadVertex
    {
        MVec2 position;
        MVec2 textureCoordinates;
    };

    void onWindowResized()
    {
        const auto frameBufferSize = window->framebufferSize();
        texturedFlatShader.setTransformationProjectionMatrix(Magnum::Matrix3::projection(Magnum::Vector2(frameBufferSize)));
    }

    void onWindowEvent(const SDL_Event& e)
    {
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) { onWindowResized(); }
    }

    Renderer(Window& window) { create(window); }

    void create(Window& window)
    {
        this->window = &window;
        
        _spriteMesh = Mesh();
        _spriteMesh.setPrimitive(Magnum::GL::MeshPrimitive::TriangleStrip);
        _spriteMesh.setCount(4);

        texturedFlatShader = ShaderFlat2D{ ShaderFlat2D::Flag::Textured };
        flatShader = ShaderFlat2D{};

        onWindowResized();
        
        window.eventSink.push_back(std::bind(&Renderer::onWindowEvent, this, std::placeholders::_1));

        setView(window.getDefaultView());
    }

    Vector2f getMouseWorldPos(const View& _view)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        const auto camPos = _view.getTopLeftPos();
        
        const auto winSize = window->windowSize();
        const float scaledX = (_view.size.x / winSize.x()) *  x;
        const float scaledY = (_view.size.y / winSize.y()) * -y;

        return { scaledX + camPos.x, scaledY + camPos.y };
    }

    Vector2i posToGridPos(Vector2f pos, Vector2f gridSize)
    {
        // Integer division truncates towards 0
        // Floor manually so it doesn't break with negative values
        return Vector2i((pos / gridSize).floored());
    }

    void setView(View view)
    {
        _view = view;
        _viewProj = Mat3::projection(MVec2{_view.size.x, _view.size.y});
    }

    View getView()
    {
        return _view;
    }

    /// Texture drawing
    // TODO: Test different BufferUsages
    Buffer vbo{ NULL, Magnum::GL::BufferUsage::DynamicDraw };
    Mesh _spriteMesh{Magnum::NoCreate};
    ShaderFlat2D texturedFlatShader{Magnum::NoCreate};
    ShaderFlat2D flatShader{Magnum::NoCreate};
    View _view;
    Mat3 _viewProj;
    Mesh _rectMesh = Magnum::MeshTools::compile(Magnum::Primitives::squareSolid());

    void drawTexture(Texture& tex, const Rectf& rect, float rot = 0, ColorRGBAf color = { 1,1,1,1 })
    {
        const QuadVertex vertices[4]
        {
            {{rect.width, rect.height}, {0.0f, 1.0f}}, /* Bottom right */
            {{rect.width, 0},           {0.0f, 0.0f}}, /* Top right */
            {{0,          rect.height}, {1.0f, 1.0f}}, /* Bottom left */
            {{0,          0},           {1.0f, 0.0f}}  /* Top left */
        };

        vbo.setData(vertices);

        const MVec2 spritePos = {rect.x, rect.y};
        const MVec2 viewPos = { _view.center.x, _view.center.y };

        const Mat3 trans = Mat3::translation(spritePos - viewPos);
        const Mat3 localrot = Mat3::rotation(-Rad(Deg(rot)));

        texturedFlatShader.setTransformationProjectionMatrix( _viewProj * trans * localrot );

        _spriteMesh.addVertexBuffer(vbo, 0, Position{}, TextureCoordinates{});
        texturedFlatShader.setColor(Magnum::Color4{color.r, color.g, color.b, color.a});
        texturedFlatShader.bindTexture(tex);

        texturedFlatShader.draw(_spriteMesh);
    }

    /// Primitives
    void drawRect(const Rectf& rect, const ColorRGBAf& color)
    {
        const MVec2 data[4]{{ 0, 0 },          { 0 + rect.width,  0},
                            { 0, rect.height}, { rect.width,  rect.height }};

        vbo.setData(data);

        Mesh mesh{Magnum::MeshPrimitive::TriangleStrip};
        mesh.setCount(4);
        mesh.addVertexBuffer(vbo, 0, ShaderFlat2D::Position{});

        const MVec2 spritePos = {rect.x, rect.y};
        const MVec2 viewPos   = { _view.center.x, _view.center.y };
        const Mat3 trans = Mat3::translation(spritePos - viewPos);

        flatShader.setTransformationProjectionMatrix(_viewProj * trans);
        flatShader.setColor({color.r, color.g, color.b, color.a});

        flatShader.draw(mesh);
    }
};
