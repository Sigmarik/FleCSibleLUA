#pragma once

#include <SFML/Graphics/Color.hpp>

namespace ecs
{
struct Position
{
    float x = 0;
    float y = 0;
};

struct BoxCollider
{
    float sizeX = 50;
    float sizeY = 50;
};

struct Velocity
{
    float x = 0;
    float y = 0;
};

struct Gravity
{
    float value = 9.815f * 50.0f;
};

struct Mass
{
    float kilos = 10.0f;
};

struct Coloration
{
    sf::Color fill{255, 151, 30};
    sf::Color outline{230, 107, 18};
};
}
