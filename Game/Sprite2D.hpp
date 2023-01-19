#pragma once

#include "Node2D.hpp"
#include "Engine.hpp"
#include "TextureManager.hpp"

struct Sprite2D : Node2D
{
    SubTexture texture;

    void getOrLoad(const String& path, TextureFiltering filtering = Texture::defaultTexFiltering, Recti region = {0,0,0,0})
    {
        Texture* texture = game::texMan.getOrLoad(path, filtering);
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
        game::renderer.drawTextureFast(texture, { gbt.pos, gbt.scale * Vector2f(texture.texture->getSize()) }, gbt.rot);
        Node2D::draw(delta);
    }
};