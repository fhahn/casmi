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
    Driver& driver_;

  public:
    RuleNode *top_rule;
    ExecutionContext& context_;

    ExecutionVisitor(ExecutionContext& context, RuleNode *init, Driver& driver);

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
    Value&& visit_expression(Expression *expr, Value& left_val, Value& right_val);
    Value&& visit_expression_single(Expression *expr, Value& val);
    Value&& visit_int_atom(IntAtom *atom) { return std::move(Value(atom->val_)); }
    Value&& visit_float_atom(FloatAtom *atom) { return std::move(Value(atom->val_)); }
    Value&& visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return std::move(Value()); }
    Value&& visit_function_atom(FunctionAtom *atom, const std::vector<Value> &expr_results);
    Value&& visit_self_atom(SelfAtom *atom) { UNUSED(atom); return std::move(Value()); }
};

class ExecutionWalker : public AstWalker<ExecutionVisitor, Value> {
  public:
    ExecutionWalker(ExecutionVisitor& v) : AstWalker<ExecutionVisitor, Value>(v) {}
    void run();
};
#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
