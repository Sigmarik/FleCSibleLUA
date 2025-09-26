#pragma once

#include <iostream>

#include "ast/visitor.h"

namespace flua::vst
{

class AstDebugger final : public ast::Visitor
{
public:
    explicit AstDebugger(std::ostream& stream)
        : m_stream(stream)
    {}

protected:
    using Visitor::visit;

    void visit(ast::Program& node) override;
    void visit(ast::Function& node) override;
    void visit(ast::System& node) override;
    void visit(ast::Loop& node) override;
    void visit(ast::Query& node) override;
    void visit(ast::Branch& node) override;
    void visit(ast::FunctionCall& node) override;
    void visit(ast::UnaryOperator& node) override;
    void visit(ast::BinaryOperator& node) override;
    void visit(ast::FieldRequest& node) override;
    void visit(ast::IndexRequest& node) override;
    void visit(ast::Constant& node) override;
    void visit(ast::Variable& node) override;
    void visit(ast::Assignment& node) override;
    void visit(ast::Return& node) override;
    void visit(ast::Break& node) override;
    void visit(ast::Continue& node) override;

private:
    void increaseIndent();
    void decreaseIndent();

    void visitList(std::vector<ast::NodePtr>& nodes);

    std::string m_indent;

    std::ostream& m_stream;
};

}
