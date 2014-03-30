#ifndef CASMI_LIBPARSE_TYPECHECK_VISITOR
#define CASMI_LIBPARSE_TYPECHECK_VISITOR

#include <utility>

#include "libparse/ast.h"
#include "libparse/visitor.h"
#include "libparse/types.h"
#include "libparse/driver.h"

class TypecheckVisitor {
  private:
    casmi_driver& driver_;

  public:
    TypecheckVisitor(casmi_driver& driver);
    void visit_specification(AstNode *spec) {}
    void visit_body_elements(AstListNode *body_elements) {}
    void visit_rule(RuleNode *rule) {}
    void visit_statement(AstNode *stmt) {}
    void visit_parblock(UnaryNode *parblock) {}
    void visit_statements(AstListNode *stmts) {}
    void visit_update(UpdateNode *update, Type t);
    Type visit_expression(Expression *expr, Type left_val, Type right_val);
    Type visit_expression_single(Expression *expr, Type val);
    Type visit_int_atom(IntAtom *atom) { return Type::INT; }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
