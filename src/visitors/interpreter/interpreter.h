#pragma once

#include <memory>

#include "ast/visitor.h"
#include "ast/data_types.h"
#include "flecsible_lua_api.h"

namespace flua::lib
{
    void print(FluaState*);
}

namespace flua::vst
{
using namespace flua;

class Interpreter final : public ast::Visitor
{
public:
    struct LuaRuntimeError : std::exception
    {
        LuaRuntimeError(const ast::INode& node, const std::string& what) : what(what), where(node.getPos()) {}

        std::string what;
        parser::CharacterPos where;
    };

    Interpreter(std::ostream& outStream, std::ostream& errStream, flecs::world* world)
        : m_errStream(errStream), m_outStream(outStream), m_world(world)
    {
        m_stack.emplace_back();
    }

    void overrideGlobal(const std::string& name, const std::function<void(FluaState*)>& function);
    void overrideGlobal(const std::string& name, double value);
    void overrideGlobal(const std::string& name, const std::string& value);

    [[nodiscard]] bool fallen() const { return m_fallen; }

protected:
    using Visitor::visit;

    void visit(ast::Program& node) override;
    void visit(ast::Function& node) override;
    void visit(ast::WhileLoop& node) override;
    void visit(ast::ForLoopNumeric& node) override;
    void visit(ast::ForLoopGeneric& node) override;
    void visit(ast::RepeatUntil& node) override;
    void visit(ast::Branch& node) override;
    void visit(ast::FunctionCall& node) override;
    void visit(ast::UnaryOperator& node) override;
    void visit(ast::BinaryOperator& node) override;
    void visit(ast::FieldRequest& node) override;
    void visit(ast::IndexRequest& node) override;
    void visit(ast::Constant& node) override;
    void visit(ast::MakeTable& node) override;
    void visit(ast::Variable& node) override;
    void visit(ast::Assignment& node) override;
    void visit(ast::LocalAssignment& node) override;
    void visit(ast::Return& node) override;
    void visit(ast::Break& node) override;
    void visit(ast::Continue& node) override;

    friend class FluaState;

private:
    friend void lib::print(FluaState* lua);

    void visitTransparentBlock(std::deque<ast::NodePtr>& nodes);

    void executeFunction(const ast::INode& node, data::Function& function, std::deque<ast::NodePtr>& args);
    void executeFunction(ast::Function& function);

    void runLuaFunction(data::LuaFunction& function, std::vector<data::GenericValue>& args);

    FluaState generatePublicState();

    struct Frame
    {
        Frame() = default;
        Frame(const Frame&) = delete;
        Frame(Frame&&) = default;

        std::unordered_map<std::string, mem_utils::CopyMovePtr<data::GenericValue>> varNameMap{};
        bool transparent = false;
    };

    struct NamespaceHolder
    {
        explicit NamespaceHolder(std::vector<Frame>& stack, bool transparent = true)
            : m_stack(&stack)
        {
            m_stack->emplace_back();
            m_stack->back().transparent = transparent;
        }

        ~NamespaceHolder() { m_stack->pop_back(); }

    private:
        std::vector<Frame>* m_stack;
    };

    std::vector<Frame> m_stack{};

    data::ValueSequence m_returnedValue{};

    bool m_breaking = false;
    bool m_continuing = false;
    bool m_returning = false;
    bool m_inLocalAssignment = false;

    bool m_fallen = false;

    std::ostream& m_errStream;
    std::ostream& m_outStream;

    std::vector<data::GenericValue> m_externalFunctionInputs{};
    std::vector<data::GenericValue> m_externalFunctionOutputs{};

    flecs::world* m_world{};
};

}
