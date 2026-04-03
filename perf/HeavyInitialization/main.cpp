#define _USE_MATH_DEFINES

#include <chrono>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

static constexpr unsigned ATTEMPT_COUNT = 100;

int main()
{
    flecs::world world;

    flua::Script script = flua::Script::Load("perf/HeavyInitialization/script.lua");
    float executionTime = INFINITY;
    for (unsigned attemptId = 0; attemptId < ATTEMPT_COUNT; ++attemptId)
    {
        std::cout << "Attempt " << attemptId << std::endl;
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        flua::DeployedScript deployment = script.deploy(world);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        float attemptTime = std::chrono::duration<float>(end - begin).count();
        executionTime = std::min(attemptTime, executionTime);
        std::cout << "Execution time: " << executionTime << " seconds" << std::endl;
    }

    std::cout << "\nApproximate script execution time: " << executionTime << std::endl;
}