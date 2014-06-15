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

class ExecutionVisitor : public BaseVisitor<const Value> {
  private:
    std::vector<Value> main_bindings;
    casm_update *add_update(const Value& val, size_t sym_id,
                            std::vector<Value> &arguments);


  public:
    std::vector<Value> value_list;
    Driver& driver_;
    ExecutionContext& context_;
    std::vector<std::vector<Value> *> rule_bindings;

    ExecutionVisitor(ExecutionContext& context, Driver& driver);

    void visit_assert(UnaryNode* assert, const Value& val);
    void visit_assure(UnaryNode* assure, const Value& val);
    void visit_update(UpdateNode *update, const Value& expr_v);
    void visit_update_dumps(UpdateNode *update, const Value& expr_v);
    void visit_call_pre(CallNode *call);
    void visit_call_pre(CallNode *call, const Value& expr);
    void visit_call(CallNode *call, std::vector<Value> &arguments);
    void visit_call_post(CallNode *call);
    void visit_print(PrintNode *node, std::vector<Value> &arguments);
    void visit_diedie(DiedieNode *node, const Value& msg);
    void visit_impossible(AstNode *node);

    void visit_let(LetNode *node, const Value& v);
    void visit_let_post(LetNode *node);
    void visit_push(PushNode *node, const Value& expr, const Value& atom);
    void visit_pop(PopNode *node, const Value& val);

    const Value visit_expression(Expression *expr, const Value& left_val,
                                 const Value& right_val);
    Value visit_expression_single(Expression *expr, const Value& val);
    const Value visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
    const Value visit_float_atom(FloatAtom *atom) { return std::move(Value(atom->val_)); }
    const Value visit_rational_atom(RationalAtom *atom) { return std::move(Value(&atom->val_)); }
    const Value visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return std::move(Value()); }
    const Value visit_function_atom(FunctionAtom *atom,
                                    std::vector<Value> &expr_results);

    const Value visit_builtin_atom(BuiltinAtom *atom, std::vector<Value> &expr_results);
    void visit_derived_function_atom_pre(FunctionAtom *atom,
                                         std::vector<Value>& arguments);
    const Value visit_derived_function_atom(FunctionAtom *atom, const Value& expr);
    const Value visit_self_atom(SelfAtom *atom) { UNUSED(atom); return Value(); }
    const Value visit_rule_atom(RuleAtom *atom) { return Value(atom->rule); }
    const Value visit_boolean_atom(BooleanAtom *atom) { return Value(atom->value); }
    const Value visit_string_atom(StringAtom *atom) { return Value(&atom->string); }
    const Value visit_list_atom(ListAtom *atom, std::vector<Value> &vals);
    const Value visit_number_range_atom(NumberRangeAtom *atom);
};

template <>
Value AstWalker<ExecutionVisitor, Value>::walk_list_atom(ListAtom *atom);

// Specialize if-then-else for ExecutionVisitor
template <>
void AstWalker<ExecutionVisitor, Value>::walk_ifthenelse(IfThenElseNode* node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_seqblock(UnaryNode* seqblock);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_parblock(UnaryNode* parblock);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_pop(PopNode* node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_push(PushNode* node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_case(CaseNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_forall(ForallNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_iterate(UnaryNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_update(UpdateNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_update_dumps(UpdateNode *node);

class ExecutionWalker : public AstWalker<ExecutionVisitor, Value> {
  private:
    std::set<std::string> initialized;

    bool init_function(const std::string& name, std::set<std::string>& visited);

  public:
    ExecutionWalker(ExecutionVisitor& v);
    void run();
};
#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
