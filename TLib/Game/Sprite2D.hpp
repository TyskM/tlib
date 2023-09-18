#pragma once

#include <TLib/Game/Node2D.hpp>
#include <TLib/Game/TextureManager.hpp>
#include <TLib/Media/Renderer2D.hpp>

struct Sprite2D : Node2D
{
    SubTexture texture;

    void getOrLoad(const String& path, TextureManager& texMan, TextureFiltering filtering = Texture::defaultTexFiltering, Recti region = {0,0,0,0})
    {
        Texture* texture = texMan.getOrLoad(path, filtering);
        if (!texture) { return; }
        setTexture(*texture, region);
    }

    void setTexture(Texture& tex, Recti region = { 0,0,0,0 })
    {
        if (region.width == 0)
        { this->texture = SubTexture(tex); }
        else
        { this->texture = SubTexture(tex, region); }
    }

    virtual void draw(float delta) override
    {
        if (texture.texture == nullptr) { return; }
        Transform2D gbt = getGlobalTransform();

        Renderer2D::drawTexture(
            texture,
            Rectf{gbt.pos, gbt.scale * Vector2f(texture.texture->getSize())},
            Renderer2D::DefaultSpriteLayer,
            ColorRGBAf::white(),
            gbt.rot);

        Node2D::draw(delta);
    }
};