#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include <utility>
#include <sys/types.h>

#include "macros.h"

#include "libsyntax/ast.h"
#include "libsyntax/visitor.h"
#include "libsyntax/driver.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class ExecutionVisitor : public BaseVisitor<value_t> {
  private:
    std::vector<value_t> main_bindings;
    casm_update *add_update(const value_t& val, size_t sym_id,
                            std::vector<value_t> &arguments);


  public:
    std::vector<value_t> value_list;
    Driver& driver_;
    ExecutionContext& context_;
    std::vector<std::vector<value_t> *> rule_bindings;

    ExecutionVisitor(ExecutionContext& context, Driver& driver);

    void visit_assert(UnaryNode* assert, const value_t& val);
    void visit_assure(UnaryNode* assure, const value_t& val);
    void visit_update(UpdateNode *update, const value_t& expr_v);
    void visit_update_subrange(UpdateNode *update, const value_t& expr_v);

    void visit_update_dumps(UpdateNode *update, const value_t& expr_v);
    void visit_call_pre(CallNode *call);
    void visit_call_pre(CallNode *call, const value_t& expr);
    void visit_call(CallNode *call, std::vector<value_t> &arguments);
    void visit_call_post(CallNode *call);
    void visit_print(PrintNode *node, std::vector<value_t> &arguments);
    void visit_diedie(DiedieNode *node, const value_t& msg);
    void visit_impossible(AstNode *node);

    void visit_let(LetNode *node, const value_t& v);
    void visit_let_post(LetNode *node);
    void visit_push(PushNode *node, const value_t& expr, const value_t& atom);
    void visit_pop(PopNode *node, const value_t& val);

    const value_t visit_expression(Expression *expr, const value_t& left_val,
                                 const value_t& right_val);
    value_t visit_expression_single(Expression *expr, const value_t& val);
    const value_t visit_int_atom(IntAtom *atom) { return std::move(value_t(atom->val_)); }
    const value_t visit_float_atom(FloatAtom *atom) { return std::move(value_t(atom->val_)); }
    const value_t visit_rational_atom(RationalAtom *atom) { return std::move(value_t(&atom->val_)); }
    const value_t visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return std::move(value_t()); }
    const value_t visit_function_atom(FunctionAtom *atom,
                                      std::vector<value_t> &expr_results);
    const value_t visit_function_atom_subrange(FunctionAtom *atom,
                                               std::vector<value_t> &expr_results);


    const value_t visit_builtin_atom(BuiltinAtom *atom, std::vector<value_t> &expr_results);
    void visit_derived_function_atom_pre(FunctionAtom *atom,
                                         std::vector<value_t>& arguments);
    const value_t visit_derived_function_atom(FunctionAtom *atom, const value_t& expr);
    const value_t visit_self_atom(SelfAtom *atom) { UNUSED(atom); return value_t(); }
    const value_t visit_rule_atom(RuleAtom *atom) { return value_t(atom->rule); }
    const value_t visit_boolean_atom(BooleanAtom *atom) { return value_t(atom->value); }
    const value_t visit_string_atom(StringAtom *atom) { return value_t(&atom->string); }
    const value_t visit_list_atom(ListAtom *atom, std::vector<value_t> &vals);
    const value_t visit_number_range_atom(NumberRangeAtom *atom);
};

template <>
value_t AstWalker<ExecutionVisitor, value_t>::walk_list_atom(ListAtom *atom);

// Specialize if-then-else for ExecutionVisitor
template <>
void AstWalker<ExecutionVisitor, value_t>::walk_ifthenelse(IfThenElseNode* node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_seqblock(UnaryNode* seqblock);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_parblock(UnaryNode* parblock);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_pop(PopNode* node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_push(PushNode* node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_case(CaseNode *node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_forall(ForallNode *node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_iterate(UnaryNode *node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_update(UpdateNode *node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_update_subrange(UpdateNode *node);

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_update_dumps(UpdateNode *node);

class ExecutionWalker : public AstWalker<ExecutionVisitor, value_t> {
  private:
    std::set<std::string> initialized;

    bool init_function(const std::string& name, std::set<std::string>& visited);

  public:
    ExecutionWalker(ExecutionVisitor& v);
    void run();
};
#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
