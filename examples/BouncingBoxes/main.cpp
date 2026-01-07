#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

#include "components.h"
#include "systems.h"

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

    world.entity()
        .set<ecs::Position>({ 400.0f, 300.0f })
        .set<ecs::BoxCollider>({ 100.0f, 100.0f })
        .set<ecs::Velocity>({ 100, 100 })
        .set<ecs::Gravity>({});

    flua::Script script = flua::Script::Load("examples/BouncingBoxes/script.lua");
    flua::DeployedScript deployment = script.deploy(world);

    while (window.isOpen())
    {
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