#include "systems.h"

#include "components.h"

void register_rendering_system(flecs::world& world, sf::RenderWindow& window, sf::Shape& shape)
{
    world.system<ecs::Position, ecs::Rotation>("Display")
        .each([&](const ecs::Position& pos, const ecs::Rotation& rotation)
        {
            shape.setPosition(sf::Vector2f(pos.x, pos.y));
            shape.setRotation(sf::radians(rotation.angle));
            window.draw(shape);
        }
    );
}
