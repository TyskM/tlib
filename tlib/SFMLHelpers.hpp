#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#define SFML
#include "DataStructures.hpp"

void setSpriteSizeInPixels(sf::Sprite& sprite, Vector2f size)
{
    if (sprite.getTexture() == NULL) throw std::runtime_error("Texture must be not NULL before calling this function.");
    auto texSize = sprite.getTexture()->getSize();
    sprite.setScale(size / Vector2f(texSize.x, texSize.y));
}