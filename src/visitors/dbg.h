#pragma once

#include <iostream>

#include "syntax/visitor.h"

namespace flua::vst
{

class AstDebugger final : public syntax::Visitor
{
public:
    explicit AstDebugger(std::ostream& stream)
        : m_stream(stream)
    {}

protected:
    using Visitor::visit;

    void visit(syntax::Program& node) override;
    void visit(syntax::Function& node) override;
    void visit(syntax::System& node) override;
    void visit(syntax::Loop& node) override;
    void visit(syntax::Query& node) override;
    void visit(syntax::Branch& node) override;
    void visit(syntax::FunctionCall& node) override;
    void visit(syntax::UnaryOperator& node) override;
    void visit(syntax::BinaryOperator& node) override;
    void visit(syntax::FieldRequest& node) override;
    void visit(syntax::IndexRequest& node) override;
    void visit(syntax::Constant& node) override;
    void visit(syntax::Variable& node) override;
    void visit(syntax::Assignment& node) override;
    void visit(syntax::Return& node) override;
    void visit(syntax::Break& node) override;
    void visit(syntax::Continue& node) override;

private:
    void increaseIndent();
    void decreaseIndent();

    void visitList(std::vector<syntax::NodePtr>& nodes);

    std::string m_indent;

    std::ostream& m_stream;
};

}
