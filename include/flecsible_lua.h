#pragma once

#include <string>
#include <set>

#include <flecs.h>

namespace flua
{

namespace syntax
{
    class Ast;
}

class Script
{
public:
    Script(const Script&);
    Script& operator=(const Script&);
    Script(Script&&);
    Script& operator=(Script&&);

    ~Script();

    static Script Parse(const std::string_view& view);

    static Script Load(const std::string& path);
    static Script Load(const std::string& path, std::string& root);

    // TODO: Add the ability to define custom c++ functions

    void deploy(flecs::world& world);

    void reload();

private:
    Script() = default;

private:
    std::set<flecs::system> m_dependentSystems;

    syntax::Ast* m_ast = nullptr;

    std::string m_sourcePath = "";
};

}
