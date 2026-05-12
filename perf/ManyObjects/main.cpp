#define _USE_MATH_DEFINES

#include <chrono>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

#include "components.h"

static constexpr unsigned UPDATE_COUNT = 1000;
static constexpr unsigned ENTITY_COUNT = 1000;

int main()
{
    flecs::world world;

    for (unsigned entityIdx = 0; entityIdx < ENTITY_COUNT; entityIdx++)
    {
        world.entity().set<ecs::Position>({0, 0}).set<ecs::Velocity>({1, 1});
    }

    flua::Script script = flua::Script::Load("perf/ManyObjects/script.lua");
    flua::DeployedScript deployment = script.deploy(world);
    long long executionTime = LLONG_MAX;
    for (unsigned attemptId = 0; attemptId < UPDATE_COUNT; ++attemptId)
    {
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        world.progress();
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        long long attemptTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        if (attemptId > 0 && attemptId % 100 == 0)
        {
            std::cout << "Tick " << attemptId << " / " << UPDATE_COUNT << ", " << attemptTime << " ms" << std::endl;
        }

        if (attemptTime == 0) continue;
        executionTime = std::min(attemptTime, executionTime);
    }

    std::cout << "\nApproximate iteration time: " << executionTime << " ms" << std::endl;
}