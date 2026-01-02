#include "flecsible_lua.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ast/ast_nodes.h"
#include "grammar/lua_parser.h"
#include "visitors/dbg.h"
#include "library/library.h"
#include "visitors/interpreter/interpreter.h"

namespace flua
{
Script::Script(const Script& other)
{
    m_ast = other.m_ast ? new ast::Ast(*other.m_ast) : nullptr;
    m_sourcePath = other.m_sourcePath;
}

Script& Script::operator=(const Script& other)
{
    if (&other == this) return *this;

    m_ast = other.m_ast ? new ast::Ast(*other.m_ast) : nullptr;
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

Script Script::Parse(const std::string& program)
{
    Script script;

    auto parsingResult = luagrmr::Parser::parse(program);

    if (parsingResult.has_value())
    {
        script.m_ast = new ast::Ast{.program = parsingResult.value()};
    }
    else
    {
        // TODO: Somehow return the error to the user?
        const auto& [what, where] = parsingResult.error();
        std::cerr << "FLua ERROR at line " << where.line << " column " << where.column << ":\n\t" << what << std::endl;
    }

    return script;
}

static std::expected<std::string, std::string> read_file_to_string(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        return std::unexpected("Could not open the file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Script Script::Load(const std::string& path)
{
    Script script;

    auto content = read_file_to_string(path);

    if (!content.has_value())
    {
        std::cerr << content.error() << std::endl;
        return script;
    }

    auto parsingResult = luagrmr::Parser::parse(*content);

    if (parsingResult.has_value())
    {
        script.m_ast = new ast::Ast{.program = parsingResult.value()};
    }
    else
    {
        // TODO: Return the error to the user?
        const auto& [what, where] = parsingResult.error();
        std::cerr << "FLua ERROR in file " << path << " at line " << where.line << " column " << where.column << ":\n\t" << what << std::endl;
    }

    return script;
}

void Script::overrideGlobal(const std::string& name, const std::function<void(FluaState*)>& function)
{
    m_globalOverrides[name] = function;
}

void Script::overrideGlobal(const std::string& name, double value)
{
    m_globalOverrides[name] = value;
}

void Script::overrideGlobal(const std::string& name, const std::string& value)
{
    m_globalOverrides[name] = value;
}

std::vector<flecs::system> Script::deploy(flecs::world& world)
{
    if (m_ast == nullptr)
    {
        return {};
    }

    vst::Interpreter interpreter(std::cout, std::cerr);

    for (const auto& [name, function] : lib::STANDARD_LIBRARY)
    {
        interpreter.overrideGlobal(name, function);
    }

    for (const auto& [name, genericValue] : m_globalOverrides)
    {
        std::visit([&](const auto& value) { interpreter.overrideGlobal(name, value); }, genericValue);
    }

    interpreter.process(*m_ast);

    return {};
}

}
