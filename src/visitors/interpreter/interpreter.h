#pragma once

#include "ast/visitor.h"
#include "ast/data_types.h"

namespace flua::vst
{

using namespace flua;

class Interpreter final : public ast::Visitor
{
public:
    explicit Interpreter(std::ostream& errStream)
        : m_errStream(errStream)
    {}

    data::GenericValue run(const std::string& function, std::vector<data::GenericValue>& params);

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

private:
    void visitBlock(std::deque<ast::NodePtr>& nodes);

    struct InternalValue
    {
        struct Ptr
        {
            unsigned stackDepth = 0;
            std::string varName{};
            std::vector<data::GenericValue> indexSequence{};
        };

        std::optional<Ptr> ptr{};
        data::GenericValue value = data::Nil{};
    };

    struct Frame
    {
        std::unordered_map<std::string, InternalValue> varNameMap{};
        bool transparent = false;
    };

    std::vector<Frame> m_stack{Frame{}};

    InternalValue m_returnedValue{};

    bool m_breaking = false;
    bool m_continuing = false;
    bool m_returning = false;

    std::ostream& m_errStream;
};

}
