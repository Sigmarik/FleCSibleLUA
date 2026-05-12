#define _USE_MATH_DEFINES

#include <chrono>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

static constexpr unsigned UPDATE_COUNT = 100;

int main()
{
    flecs::world world;

    flua::Script script = flua::Script::Load("perf/HeavyInitialization/script.lua");
    long long executionTime = LLONG_MAX;
    for (unsigned attemptId = 0; attemptId < UPDATE_COUNT; ++attemptId)
    {
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        flua::DeployedScript deployment = script.deploy(world);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        long long attemptTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        executionTime = std::min(attemptTime, executionTime);
        if (attemptId > 0 && attemptId % 10 == 0)
        {
            std::cout << "Attempt " << attemptId << "/" << UPDATE_COUNT << ", " << executionTime << " ms" << std::endl;
        }
    }

    std::cout << "\nApproximate script execution time: " << executionTime << " ms" << std::endl;
}