#ifndef CASMI_LIBPARSE_TYPECHECK_VISITOR
#define CASMI_LIBPARSE_TYPECHECK_VISITOR

#include <utility>

#include "macros.h"

#include "libsyntax/ast.h"
#include "libsyntax/visitor.h"
#include "libsyntax/types.h"
#include "libsyntax/driver.h"

class TypecheckVisitor {
  private:
    Driver& driver_;

  public:
    TypecheckVisitor(Driver& driver);

    void visit_specification(AstNode *spec) { UNUSED(spec); }
    void visit_init(AstNode *init) { UNUSED(init); }
    void visit_body_elements(AstListNode *body_elements) { UNUSED(body_elements); }
    void visit_function_def(FunctionDefNode *def,
                            const std::vector<std::pair<Type, Type>>& initializers);
    void visit_rule(RuleNode *rule) { UNUSED(rule); }
    void visit_statement(AstNode *stmt) { UNUSED(stmt); }
    void visit_parblock(UnaryNode *parblock) { UNUSED(parblock); }
    void visit_statements(AstListNode *stmts) { UNUSED(stmts); }
    void visit_ifthenelse(IfThenElseNode *node, Type cond);
    void visit_assert(UnaryNode *assert, Type t);
    void visit_update(UpdateNode *update, Type func, Type expr);
    Type visit_expression(Expression *expr, Type left_val, Type right_val);
    Type visit_expression_single(Expression *expr, Type val);
    Type visit_int_atom(IntAtom *atom) { UNUSED(atom); return Type::INT; }
    Type visit_float_atom(FloatAtom *atom) { UNUSED(atom); return Type::FLOAT; }
    Type visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return Type::UNDEF; }
    Type visit_function_atom(FunctionAtom *atom,
                             const std::vector<Type> &expr_results);
    Type visit_self_atom(SelfAtom *atom) { UNUSED(atom); return Type::SELF; }
    Type visit_rule_atom(RuleAtom *atom);
    Type visit_boolean_atom(BooleanAtom *atom) { UNUSED(atom); return Type::BOOLEAN; }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
