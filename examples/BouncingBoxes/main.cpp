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
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Bouncing Boxes", sf::Style::Default, sf::State::Windowed, settings);

    float boxSize = 200;
    sf::RectangleShape box(sf::Vector2f(boxSize, boxSize));
    box.setOrigin(sf::Vector2f(boxSize / 2.0f, boxSize / 2.0f));
    box.setOutlineThickness(10);

    flecs::world world;
    register_rendering_system(world, window, box);

    world.entity()
        .set<ecs::Position>({ 200.0f, 300.0f })
        .set<ecs::BoxCollider>({ 100.0f, 100.0f })
        .set<ecs::Velocity>({ 100, 100 })
        .set<ecs::Mass>({ 50.0f })
        .set<ecs::Gravity>({})
        .set<ecs::Coloration>({{255, 151, 30}, {230, 107, 18}});

    world.entity()
        .set<ecs::Position>({ 600.0f, 200.0f })
        .set<ecs::BoxCollider>({ 150.0f, 150.0f })
        .set<ecs::Mass>({ 100.0f })
        .set<ecs::Velocity>({ 300, 123 })
        .set<ecs::Coloration>({{255, 151, 30}, {230, 107, 18}});

    flua::Script script = flua::Script::Load("examples/BouncingBoxes/script.lua");
    script.overrideGlobal("SCREEN_X", window.getSize().x);
    script.overrideGlobal("SCREEN_Y", window.getSize().y);
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