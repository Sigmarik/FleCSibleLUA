#pragma once

#include <string>

#include <flecs.h>
#include <vector>
#include <functional>

namespace flua
{

class FluaState;

namespace ast
{
    class Ast;
}

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

    std::vector<flecs::system> deploy(flecs::world& world);

    void import_library_function(const std::string& name, const std::function<void(FluaState*)>& func);

private:
    Script() = default;

private:
    ast::Ast* m_ast = nullptr;

    std::string m_sourcePath{};
};

}
