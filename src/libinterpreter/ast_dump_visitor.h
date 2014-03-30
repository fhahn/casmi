#ifndef CASMI_LIBINTERPRETER_AST_DUMP_VISITOR
#define CASMI_LIBINTERPRETER_AST_DUMP_VISITOR

#include <utility>
#include <string>

#include "libparse/ast.h"
#include "libparse/visitor.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class AstDumpVisitor {
  private:
    std::string output;

  public:
    void visit_specification(AstNode *spec) {}
    void visit_body_elements(AstListNode *body_elements) {}
    void visit_rule(RuleNode *rule) {}
    void visit_statement(AstNode *stmt) {}
    void visit_parblock(UnaryNode *parblock) {}
    void visit_statements(AstListNode *stmts) {}
    void visit_update(UpdateNode *update, Value& val);
    Value&& visit_expression(Expression *expr, Value& left_val, Value& right_val);
    Value&& visit_expression_single(Expression *expr, Value& val);
    Value&& visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
