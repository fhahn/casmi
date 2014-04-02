#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include <utility>

#include "macros.h"

#include "libparse/ast.h"
#include "libparse/visitor.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class ExecutionVisitor {
  private:
    AstNode *root_;
    ExecutionContext& context_;

  public:
    ExecutionVisitor(AstNode *root, ExecutionContext& context);

    void visit_specification(AstNode *spec) { UNUSED(spec); }
    void visit_body_elements(AstListNode *body_elements) { UNUSED(body_elements); }
    void visit_rule(RuleNode *rule) { UNUSED(rule); }
    void visit_statement(AstNode *stmt) { UNUSED(stmt); }
    void visit_parblock(UnaryNode *parblock) { UNUSED(parblock); }
    void visit_statements(AstListNode *stmts) { UNUSED(stmts); }
    void visit_update(UpdateNode *update, Value& val);
    Value&& visit_expression(Expression *expr, Value& left_val, Value& right_val);
    Value&& visit_expression_single(Expression *expr, Value& val);
    Value&& visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
