#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

#include "components.h"
#include "systems.h"

static void system_action_test(ecs_iter_t *it)
{
    std::cout << "TEST ACTION" << std::endl;
}

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Rotating Box", sf::Style::Default, sf::State::Windowed, settings);

    float boxSize = 200;
    sf::RectangleShape box(sf::Vector2f(boxSize, boxSize));
    box.setOrigin(sf::Vector2f(boxSize / 2.0f, boxSize / 2.0f));
    box.setPosition(sf::Vector2f(400, 300));
    box.setFillColor(sf::Color(255, 151, 30));
    box.setOutlineColor(sf::Color(230, 107, 18));
    box.setOutlineThickness(10);

    flecs::world world;
    register_rendering_system(world, window, box);

    flecs::entity boxEntity = world.entity()
        .set<Position>({ 400.0f, 300.0f })
        .set<Rotation>({ 12.3f });

    flua::Script script = flua::Script::Load("examples/RotatingSquare/script.lua");
    script.overrideGlobal("ROTATION_SPEED", 7.0);
    script.overrideGlobal("BOX_ENTITY_ID", static_cast<double>(boxEntity.id()));
    script.deploy(world);

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color(36, 45, 51));

        world.progress();

        window.display();
    }
}