#include "systems.h"

#include "components.h"

void register_rendering_system(flecs::world& world, sf::RenderWindow& window, sf::RectangleShape& shape)
{
    world.system<ecs::Position, ecs::BoxCollider, ecs::Coloration>("Display")
        .each([&](const ecs::Position& pos, const ecs::BoxCollider& collider, const ecs::Coloration& color)
        {
            shape.setPosition(sf::Vector2f(pos.x, pos.y));
            shape.setSize(sf::Vector2f(collider.sizeX, collider.sizeY));
            shape.setOrigin(sf::Vector2f(collider.sizeX, collider.sizeY) / 2.0f);
            shape.setFillColor(sf::Color(color.fill.r, color.fill.g, color.fill.b));
            shape.setOutlineColor(sf::Color(color.outline.r, color.outline.g, color.outline.b));
            window.draw(shape);
        }
    );
}
