#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include "libparse/ast.h"
#include "libparse/visitor.h"
#include "libinterpreter/execution_context.h"

class ExecutionVisitor {
  private:
    AstNode *root_;
    ExecutionContext& context_;

  public:
    ExecutionVisitor(AstNode *root, ExecutionContext& context);

    void visit_specification(AstNode *spec) {}
    void visit_body_elements(AstListNode *body_elements) {}
    void visit_rule(RuleNode *rule) {}
    void visit_statement(AstNode *stmt) {}
    void visit_parblock(UnaryNode *parblock) {}
    void visit_statements(AstListNode *stmts) {}
    void visit_update(UpdateNode *update);
    void visit_expression(Expression *expr) {}
    void visit_int_atom(IntAtom *atom) {}
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
