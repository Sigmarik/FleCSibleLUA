#pragma once

#include <string>

#include <flecs.h>
#include <vector>

namespace flua
{

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

    // TODO: Add the ability to define custom functions

    std::vector<flecs::system> deploy(flecs::world& world);

private:
    Script() = default;

private:
    ast::Ast* m_ast = nullptr;

    std::string m_sourcePath{};
};

}
