#include <filesystem>
#include <flecs.h>
#include <flecsible_lua.h>
#include <iostream>

#include "teestream.h"

int main()
{
    flecs::world world;

    std::cout << "[SYSTEM] Running vanilla lua interpreter torture" << std::endl;

    const std::filesystem::path scriptsDirectory = "test/VanillaLuaTorture/scripts";

    std::vector<std::string> failedTests;

    for (const auto& scriptFile : std::filesystem::directory_iterator(scriptsDirectory))
    {
        if (!scriptFile.is_regular_file())
            continue;

        std::stringstream testOut;
        std::stringstream testErr;
        teestream outStream(std::cout, testOut);
        teestream errStream(std::cerr, testErr);

        std::string testPath = scriptFile.path().string();
        std::cout << "[SYSTEM] Running " << testPath << std::endl;
        flua::Script script = flua::Script::Load(testPath);
        flua::DeployedScript deployment = script.deploy(world, outStream, errStream);

        if (!testErr.str().empty() || testOut.str().contains("FAIL"))
        {
            std::cout << "[SYSTEM] Test FAILED!" << std::endl;
            failedTests.push_back(testPath);
        }
    }

    if (failedTests.empty())
    {
        std::cout << "[SYSTEM] All tests PASSED" << std::endl;
    }
    else
    {
        std::cout << "[SYSTEM] " << failedTests.size() << " tests FAILED:" << std::endl;
        for (const auto& failedTest : failedTests)
        {
            std::cout << "[SYSTEM]   " << failedTest << std::endl;
        }
    }
}