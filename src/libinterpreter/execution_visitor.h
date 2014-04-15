#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include <utility>

#include "macros.h"

#include "libsyntax/ast.h"
#include "libsyntax/visitor.h"
#include "libsyntax/driver.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class ExecutionVisitor {
  private:

    std::vector<Value> value_list;

  public:
    Driver& driver_;
    ExecutionContext& context_;

    ExecutionVisitor(ExecutionContext& context, Driver& driver);

    void visit_specification(AstNode *spec) { UNUSED(spec); }
    void visit_init(AstNode *init) { UNUSED(init); }
    void visit_body_elements(AstListNode *body_elements) { UNUSED(body_elements); }
    void visit_function_def(FunctionDefNode *def) { UNUSED(def); }
    void visit_rule(RuleNode *rule) { UNUSED(rule); }
    void visit_statement(AstNode *stmt) { UNUSED(stmt); }
    void visit_parblock(UnaryNode *parblock) { UNUSED(parblock); }
    void visit_assert(UnaryNode* assert, Value& val);
    void visit_statements(AstListNode *stmts) { UNUSED(stmts); }
    void visit_update(UpdateNode *update, Value& func_val, Value& expr_v);
    void visit_call_pre(CallNode *call);
    void visit_call_pre(CallNode *call, Value& expr);
    void visit_call(CallNode *call, std::vector<Value> &arguments);
    void visit_print(PrintNode *node, const std::vector<Value> &arguments);
    Value&& visit_expression(Expression *expr, Value& left_val, Value& right_val);
    Value&& visit_expression_single(Expression *expr, Value& val);
    Value&& visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
    Value&& visit_float_atom(FloatAtom *atom) { return std::move(Value(atom->val_)); }
    Value&& visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return std::move(Value()); }
    Value&& visit_function_atom(FunctionAtom *atom, std::vector<Value> &expr_results);
    Value&& visit_self_atom(SelfAtom *atom) { UNUSED(atom); return std::move(Value()); }
    Value&& visit_rule_atom(RuleAtom *atom) { return std::move(Value(atom->rule)); }
    Value&& visit_boolean_atom(BooleanAtom *atom) { return std::move(Value(atom->value)); }
    Value&& visit_string_atom(StringAtom *atom) { return std::move(Value(&atom->string)); }
};

// Specialize if then else for ExecutionVisitor
template <>
void AstWalker<ExecutionVisitor, Value>::walk_ifthenelse(IfThenElseNode* node);



class ExecutionWalker : public AstWalker<ExecutionVisitor, Value> {
  public:
    ExecutionWalker(ExecutionVisitor& v) : AstWalker<ExecutionVisitor, Value>(v) {}
    void run();
};
#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
