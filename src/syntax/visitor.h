#pragma once

#include "ast_nodes.h"

namespace flua::syntax
{

class Visitor
{
public:
    void process(Ast& ast);

    virtual ~Visitor() = default;

protected:
    void visit(AstNode& node);
    void visit(NodePtr& ptr);

    virtual void visit(Program& node) = 0;
    virtual void visit(Function& node) = 0;
    virtual void visit(System& node) = 0;
    virtual void visit(Loop& node) = 0;
    virtual void visit(Query& node) = 0;
    virtual void visit(Branch& node) = 0;
    virtual void visit(FunctionCall& node) = 0;
    virtual void visit(UnaryOperator& node) = 0;
    virtual void visit(BinaryOperator& node) = 0;
    virtual void visit(FieldRequest& node) = 0;
    virtual void visit(IndexRequest& node) = 0;
    virtual void visit(Constant& node) = 0;
    virtual void visit(Variable& node) = 0;
    virtual void visit(Assignment& node) = 0;
    virtual void visit(Return& node) = 0;
    virtual void visit(Break& node) = 0;
    virtual void visit(Continue& node) = 0;
};

}
