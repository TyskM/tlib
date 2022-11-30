#pragma once

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

class SFMLBox2DDebugDraw : public b2Draw
{
protected:
    sf::RenderWindow* window;

public:
    static inline float scale = 1.f;

    SFMLBox2DDebugDraw(sf::RenderWindow& window) : window(&window) {};

    static sf::Color GLColorToSFML(const b2Color &color, sf::Uint8 alpha = 255)
    {
        return sf::Color(static_cast<sf::Uint8>(color.r * 255), static_cast<sf::Uint8>(color.g * 255), static_cast<sf::Uint8>(color.b * 255), alpha);
    }

    static sf::Vector2f B2VecToSFVec(const b2Vec2 &vector, bool scaleToPixels = true)
    {
        return sf::Vector2f(vector.x * (scaleToPixels ? scale : 1.f), vector.y * (scaleToPixels ? scale : 1.f));
    }

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        sf::ConvexShape polygon(vertexCount);
        //sf::Vector2f center;
        for(int i = 0; i < vertexCount; i++)
        {
            //polygon.setPoint(i, SFMLDraw::B2VecToSFVec(vertices[i]));
            sf::Vector2f transformedVec = SFMLBox2DDebugDraw::B2VecToSFVec(vertices[i]);
            polygon.setPoint(i, sf::Vector2f(std::floor(transformedVec.x), std::floor(transformedVec.y))); // flooring the coords to fix distorted lines on flat surfaces
        }																								   // they still show up though.. but less frequently
        polygon.setOutlineThickness(-1.f);
        polygon.setFillColor(sf::Color::Transparent);
        polygon.setOutlineColor(SFMLBox2DDebugDraw::GLColorToSFML(color));

        window->draw(polygon);
    }

    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        sf::ConvexShape polygon(vertexCount);
        for(int i = 0; i < vertexCount; i++)
        {
            //polygon.setPoint(i, SFMLDraw::B2VecToSFVec(vertices[i]));
            sf::Vector2f transformedVec = SFMLBox2DDebugDraw::B2VecToSFVec(vertices[i]);
            polygon.setPoint(i, sf::Vector2f(std::floor(transformedVec.x), std::floor(transformedVec.y))); // flooring the coords to fix distorted lines on flat surfaces
        }																								   // they still show up though.. but less frequently
        polygon.setOutlineThickness(-1.f);
        polygon.setFillColor(SFMLBox2DDebugDraw::GLColorToSFML(color, 60));
        polygon.setOutlineColor(SFMLBox2DDebugDraw::GLColorToSFML(color));

        window->draw(polygon);
    }

    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override
    {
        sf::CircleShape circle(radius * scale);
        circle.setOrigin(radius * scale, radius * scale);
        circle.setPosition(SFMLBox2DDebugDraw::B2VecToSFVec(center));
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(-1.f);
        circle.setOutlineColor(SFMLBox2DDebugDraw::GLColorToSFML(color));

        window->draw(circle);
    }

    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override
    {
        sf::CircleShape circle(radius * scale);
        circle.setOrigin(radius * scale, radius * scale);
        circle.setPosition(SFMLBox2DDebugDraw::B2VecToSFVec(center));
        circle.setFillColor(SFMLBox2DDebugDraw::GLColorToSFML(color, 60));
        circle.setOutlineThickness(1.f);
        circle.setOutlineColor(SFMLBox2DDebugDraw::GLColorToSFML(color));

        b2Vec2 endPoint = center + radius * axis;
        sf::Vertex line[2] = 
        {
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(center), SFMLBox2DDebugDraw::GLColorToSFML(color)),
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(endPoint), SFMLBox2DDebugDraw::GLColorToSFML(color)),
        };

        window->draw(circle);
        window->draw(line, 2, sf::Lines);
    }

    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
    {
        sf::Vertex line[] =
        {
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(p1), SFMLBox2DDebugDraw::GLColorToSFML(color)),
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(p2), SFMLBox2DDebugDraw::GLColorToSFML(color))
        };

        window->draw(line, 2, sf::Lines);
    }

    void DrawTransform(const b2Transform& xf) override
    {
        float lineLength = 0.4;

        /*b2Vec2 xAxis(b2Vec2(xf.p.x + (lineLength * xf.q.c), xf.p.y + (lineLength * xf.q.s)));*/
        b2Vec2 xAxis = xf.p + lineLength * xf.q.GetXAxis();
        sf::Vertex redLine[] = 
        {
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(xf.p), sf::Color::Red),
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(xAxis), sf::Color::Red)
        };

        /*b2Vec2 yAxis(b2Vec2(xf.p.x + (lineLength * -xf.q.s), xf.p.y + (lineLength * xf.q.c)));*/
        b2Vec2 yAxis = xf.p + lineLength * xf.q.GetYAxis();
        sf::Vertex greenLine[] = 
        {
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(xf.p), sf::Color::Green),
            sf::Vertex(SFMLBox2DDebugDraw::B2VecToSFVec(yAxis), sf::Color::Green)
        };

        window->draw(redLine, 2, sf::Lines);
        window->draw(greenLine, 2, sf::Lines);
    }

    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override { return; };
};