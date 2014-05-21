#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include <utility>

#include "macros.h"

#include "libsyntax/ast.h"
#include "libsyntax/visitor.h"
#include "libsyntax/driver.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class ExecutionVisitor : public BaseVisitor<Value> {
  private:

    std::vector<Value> main_bindings;
    casm_update *add_update(const Value& val, size_t sym_id,
                          const std::vector<Value> &arguments);

  public:
    std::vector<bool> forks;
    std::vector<Value> value_list;
    Driver& driver_;
    ExecutionContext& context_;
    std::vector<std::vector<Value> *> rule_bindings;

    ExecutionVisitor(ExecutionContext& context, Driver& driver);

    void visit_assert(UnaryNode* assert, Value& val);
    void visit_update(UpdateNode *update, Value& expr_v);
    void visit_update_dumps(UpdateNode *update, Value& expr_v);
    void visit_call_pre(CallNode *call);
    void visit_call_pre(CallNode *call, Value& expr);
    void visit_call(CallNode *call, std::vector<Value> &arguments);
    void visit_call_post(CallNode *call);
    void visit_print(PrintNode *node, const std::vector<Value> &arguments);
    void visit_diedie(DiedieNode *node, const Value& msg);

    void visit_parblock(UnaryNode*);
    void visit_parblock_post();
    void visit_seqblock(UnaryNode*);
    void visit_seqblock_post();

    void visit_forall_post();

    void visit_let(LetNode *node, Value& v);
    void visit_let_post(LetNode *node);
    void visit_push(PushNode *node, const Value& expr, const Value& atom);
    void visit_pop(PopNode *node, const Value& val);

    Value visit_expression(Expression *expr, Value& left_val, Value& right_val);
    Value visit_expression_single(Expression *expr, Value& val);
    Value visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
    Value visit_float_atom(FloatAtom *atom) { return std::move(Value(atom->val_)); }
    Value visit_rational_atom(RationalAtom *atom) { return std::move(Value(&atom->val_)); }
    Value visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return std::move(Value()); }
    Value visit_function_atom(FunctionAtom *atom, std::vector<Value> &expr_results);

    Value visit_builtin_atom(BuiltinAtom *atom, std::vector<Value> &expr_results);
    void visit_derived_function_atom_pre(FunctionAtom *atom, std::vector<Value>& arguments);
    Value visit_derived_function_atom(FunctionAtom *atom, Value& expr);
    Value visit_self_atom(SelfAtom *atom) { UNUSED(atom); return Value(); }
    Value visit_rule_atom(RuleAtom *atom) { return Value(atom->rule); }
    Value visit_boolean_atom(BooleanAtom *atom) { return Value(atom->value); }
    Value visit_string_atom(StringAtom *atom) { return Value(&atom->string); }
    Value visit_list_atom(ListAtom *atom, std::vector<Value> &vals);
    Value visit_number_range_atom(NumberRangeAtom *atom);
};

// Specialize if-then-else for ExecutionVisitor
template <>
void AstWalker<ExecutionVisitor, Value>::walk_ifthenelse(IfThenElseNode* node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_case(CaseNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_forall(ForallNode *node);

template <>
void AstWalker<ExecutionVisitor, Value>::walk_iterate(UnaryNode *node);

class ExecutionWalker : public AstWalker<ExecutionVisitor, Value> {
  private:
    std::set<std::string> initialized;
    bool init_function(const std::string& name, std::set<std::string>& visited);

  public:
    ExecutionWalker(ExecutionVisitor& v) : AstWalker<ExecutionVisitor, Value>(v), initialized() {}
    void run();
};
#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
