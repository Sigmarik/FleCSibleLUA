#include "systems.h"

#include "components.h"

void register_rendering_system(flecs::world& world, sf::RenderWindow& window, sf::Shape& shape)
{
    world.system<Position, Rotation>("Display")
        .each([&](const Position& pos, const Rotation& rotation)
        {
            shape.setPosition(sf::Vector2f(pos.x, pos.y));
            shape.setRotation(sf::radians(rotation.angle));
            window.draw(shape);
        }
    );
}
