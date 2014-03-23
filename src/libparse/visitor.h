#ifndef CASMI_LIBPARSE_VISITOR_H
#define CASMI_LIBPARSE_VISITOR_H

#include "libparse/ast.h"

class AstNode;

class AstVisitor {
  public:
    // main entry point
    void walk_specification(AstNode *spec);

    void visit_specification(AstNode *spec) {}
    void visit_body_elements(AstListNode *body_elements) {}
    void visit_rule(UnaryNode *rule) {}
    void visit_statement(AstNode *stmt) {}
    void visit_parblock(UnaryNode *parblock) {}
    void visit_statements(AstListNode *stmts) {}
    void visit_update(UpdateNode *update) {}
    void visit_expression(Expression *expr) {}
    void visit_int_atom(IntAtom *atom) {}

  protected:
    void walk_body_elements(AstListNode *body_elements);
    void walk_rule(UnaryNode *rule);
    void walk_statement(AstNode *stmt);
    void walk_parblock(UnaryNode *parblock);
    void walk_statements(AstListNode *stmts);
    void walk_update(UpdateNode *update);
    void walk_expression(Expression *expr);
    void walk_atom(AtomNode *atom);

};

#endif
