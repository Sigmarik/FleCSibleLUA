#pragma once

#include <string>

#include <flecs.h>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

namespace flua
{

class FluaState;

namespace vst
{
    class Interpreter;
}

namespace ast
{
    class Ast;
}

struct DeployedScript
{
    ~DeployedScript();
    DeployedScript(const DeployedScript&) = delete;
    DeployedScript(DeployedScript&&) noexcept ;
    DeployedScript& operator=(const DeployedScript&) = delete;
    DeployedScript& operator=(DeployedScript&&) noexcept ;

    void overrideGlobal(const std::string& name, const std::function<void(FluaState&)>& function);
    void overrideGlobal(const std::string& name, double value);
    void overrideGlobal(const std::string& name, const std::string& value);

    std::optional<double> getGlobalNumber(const std::string& name);
    std::optional<std::string> getGlobalString(const std::string& name);

    friend class Script;

private:
    DeployedScript() = default;

    vst::Interpreter* m_interpreter = nullptr;
};

class Script
{
public:
    Script(const Script&);
    Script& operator=(const Script&);
    Script(Script&&) noexcept;
    Script& operator=(Script&&) noexcept;

    ~Script();

    static Script Parse(const std::string& view);

    static Script Load(const std::string& path);

    void overrideGlobal(const std::string& name, const std::function<void(FluaState&)>& function);
    void overrideGlobal(const std::string& name, double value);
    void overrideGlobal(const std::string& name, const std::string& value);

    [[nodiscard]] DeployedScript deploy(flecs::world& world, std::ostream& outStream = std::cout,
        std::ostream& errStream = std::cerr);

private:
    Script() = default;

private:
    ast::Ast* m_ast = nullptr;

    std::string m_sourcePath{};

    using PrimitiveGeneric = std::variant<std::function<void(FluaState&)>, std::string, double>;
    std::unordered_map<std::string, PrimitiveGeneric> m_globalOverrides{};
};

}
