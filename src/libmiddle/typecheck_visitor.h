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
    void check_numeric_operator(const yy::location& loc,  Type *type,
                                const Expression::Operation op);

    std::vector<std::vector<Type*> *> rule_binding_types;
    std::vector<std::map<std::string, size_t> *> rule_binding_offsets;

  public:
    TypecheckVisitor(Driver& driver);

    void visit_specification(AstNode *spec) { UNUSED(spec); }
    void visit_init(AstNode *init) { UNUSED(init); }
    void visit_body_elements(AstListNode *body_elements) { UNUSED(body_elements); }
    void visit_function_def(FunctionDefNode *def,
                            const std::vector<std::pair<Type*, Type*>>& initializers);
    void visit_derived_def_pre(FunctionDefNode *def);
    void visit_derived_def(FunctionDefNode *def, Type* expr);

    void visit_rule(RuleNode *rule);
    void visit_statement(AstNode *stmt) { UNUSED(stmt); }
    void visit_seqblock(UnaryNode *seqblock) { UNUSED(seqblock); }
    void visit_parblock(UnaryNode *parblock) { UNUSED(parblock); }
    void visit_statements(AstListNode *stmts) { UNUSED(stmts); }
    void visit_ifthenelse(IfThenElseNode *node, Type* cond);
    void visit_assert(UnaryNode *assert, Type* t);
    void visit_update(UpdateNode *update, Type* func, Type* expr);
    void visit_call_pre(CallNode *call);
    void visit_call_pre(CallNode *call, Type* expr);
    void visit_call(CallNode *call, std::vector<Type*>& argument_results);
    void visit_call_post(CallNode *call);
    void visit_print(PrintNode*, std::vector<Type*>&) {}

    void visit_let(LetNode *node, Type* v);
    void visit_let_post(LetNode *node);
    void visit_push(PushNode *node, Type *expr, Type *atom);
    void visit_pop(PopNode *node);
    void visit_case(CaseNode *node, Type *val, const std::vector<Type*>& case_labels);

    void visit_forall_pre(ForallNode *node);
    void visit_forall_post(ForallNode *node);

    Type* visit_expression(Expression *expr, Type* left_val, Type* right_val);
    Type* visit_expression_single(Expression *expr, Type* val);
    Type* visit_int_atom(IntAtom *atom) { return &atom->type_; }
    Type* visit_float_atom(FloatAtom *atom) {  return &atom->type_; }
    Type* visit_undef_atom(UndefAtom *atom) { return &atom->type_; }
    Type* visit_function_atom(FunctionAtom *atom,
                             const std::vector<Type*> &expr_results);
   Type* visit_builtin_atom(BuiltinAtom *atom,
                             const std::vector<Type*> &expr_results);

    void visit_derived_function_atom_pre(FunctionAtom *atom);
    Type* visit_derived_function_atom(FunctionAtom *atom,
                             const std::vector<Type*> &expr_results, Type* expr);
    Type* visit_self_atom(SelfAtom *atom) { return &atom->type_;  }
    Type* visit_rule_atom(RuleAtom *atom);
    Type* visit_boolean_atom(BooleanAtom *atom) { return &atom->type_; }
    Type* visit_string_atom(StringAtom *atom) { return &atom->type_; }
    Type* visit_list_atom(ListAtom *atom, std::vector<Type*> &vals);
    Type* visit_number_range_atom(NumberRangeAtom *atom) { return &atom->type_; }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
