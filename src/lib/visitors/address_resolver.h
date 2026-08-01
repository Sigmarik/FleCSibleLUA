#pragma once

#include <iostream>

#include "ast/visitor.h"

namespace flua::vst
{

class AddressResolver final : public ast::Visitor
{
protected:
    using Visitor::visit;

    void visit(ast::Program& node) override;
    void visit(ast::Function& node) override;
    void visit(ast::System& node) override;
    void visit(ast::WhileLoop& node) override;
    void visit(ast::DoBlock& node) override;
    void visit(ast::ForLoopNumeric& node) override;
    void visit(ast::ForLoopGeneric& node) override;
    void visit(ast::Query& node) override;
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
    void visitList(std::deque<ast::NodePtr>& nodes);

    data::Address resolveLocal(const mem_utils::PointerMappedString& name);
    std::optional<data::Address> resolveUnknown(const mem_utils::PointerMappedString& name);

    bool m_canDefineGlobals = false;

    unsigned m_currentAddress = 0;
    std::vector<std::map<mem_utils::PointerMappedString, data::Address>> m_stack{{}};
};

}
