#pragma once

#include <string>

#include <flecs.h>
#include <vector>
#include <functional>
#include <memory>
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

    void overrideGlobal(const std::string& name, const std::function<void(FluaState*)>& function);
    void overrideGlobal(const std::string& name, double value);
    void overrideGlobal(const std::string& name, const std::string& value);

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

    void overrideGlobal(const std::string& name, const std::function<void(FluaState*)>& function);
    void overrideGlobal(const std::string& name, double value);
    void overrideGlobal(const std::string& name, const std::string& value);

    [[nodiscard]] DeployedScript deploy(flecs::world& world);

private:
    Script() = default;

private:
    ast::Ast* m_ast = nullptr;

    std::string m_sourcePath{};

    using PrimitiveGeneric = std::variant<std::function<void(FluaState*)>, std::string, double>;
    std::unordered_map<std::string, PrimitiveGeneric> m_globalOverrides{};
};

}
