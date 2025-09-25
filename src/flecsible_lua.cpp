#include "flecsible_lua.h"

#include <iostream>

#include "syntax/ast_nodes.h"
#include "grammar/math.h"

namespace flua
{
Script::Script(const Script& other)
{
    m_ast = other.m_ast ? new syntax::Ast(*other.m_ast) : nullptr;
    m_sourcePath = other.m_sourcePath;
}

Script& Script::operator=(const Script& other)
{
    if (&other == this) return *this;

    m_ast = other.m_ast ? new syntax::Ast(*other.m_ast) : nullptr;
    m_sourcePath = other.m_sourcePath;
    return *this;
}

Script::Script(Script&& other) noexcept
{
    m_ast = other.m_ast;
    other.m_ast = nullptr;

    m_sourcePath = std::move(other.m_sourcePath);
}

Script& Script::operator=(Script&& other) noexcept
{
    m_ast = other.m_ast;
    other.m_ast = nullptr;

    m_sourcePath = std::move(other.m_sourcePath);

    return *this;
}

Script::~Script()
{
    delete m_ast;
    m_ast = nullptr;
}

Script Script::Parse(const std::string_view& view)
{
    // TODO: Implement

    return Script();
}

Script Script::Load(const std::string& path)
{
    // TODO: Implement

    return Script();
}

std::vector<flecs::system> Script::deploy(flecs::world& world)
{
    // TODO: Implement

    std::cout << "Parsing result: " << grammar::solve_something();

    return {};
}

}
