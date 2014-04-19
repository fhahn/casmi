#ifndef CASMI_LIBINTERPRETER_AST_DUMP_VISITOR
#define CASMI_LIBINTERPRETER_AST_DUMP_VISITOR

#include <utility>
#include <string>
#include <sstream>

#include "libsyntax/ast.h"
#include "libsyntax/visitor.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

class AstDumpVisitor {
  private:
    std::stringstream dump_stream_;

    void dump_node(uint64_t key, const std::string& name);
    void dump_node(AstNode *n, const std::string& name);
    void dump_link(uint64_t from, uint64_t to);
    void dump_link(AstNode *from, AstNode *to);


  public:
    AstDumpVisitor();
    std::string get_dump();

    void visit_specification(AstNode *spec) { UNUSED(spec); }
    void visit_init(AstNode *init);
    void visit_body_elements(AstListNode *body_elements);
    void visit_function_def(FunctionDefNode *def, const std::vector<std::pair<bool, bool>>&);
    void visit_derived_def_pre(FunctionDefNode *def) {}
    void visit_derived_def(FunctionDefNode *def, bool);
    void visit_rule(RuleNode *rule);
    void visit_statements(AstListNode *stmts);
    void visit_statement(AstNode *stmt);
    void visit_ifthenelse(IfThenElseNode *node, bool);
    bool visit_assert(UnaryNode *assert, bool);
    void visit_seqblock(UnaryNode *seqblock);
    void visit_parblock(UnaryNode *parblock);
    bool visit_update(UpdateNode *update, bool, bool);
    bool visit_call_pre(CallNode *call);
    bool visit_call_pre(CallNode *call, bool);
    bool visit_call(CallNode *call, std::vector<bool>& argument_results);
    void visit_call_post(CallNode *call) {UNUSED(call);}
    bool visit_print(PrintNode *node, std::vector<bool>& argument_results);
    bool visit_expression(Expression *expr, bool, bool);
    bool visit_expression_single(Expression *expr, bool);
    bool visit_int_atom(IntAtom *atom);
    bool visit_float_atom(FloatAtom *atom);
    bool visit_undef_atom(UndefAtom *atom) { UNUSED(atom); return true; }
    bool visit_function_atom(FunctionAtom *atom,
                             const std::vector<bool> &expr_results);
    bool visit_derived_function_atom(FunctionAtom *atom, 
                         const std::vector<bool> &expr_results, bool) {}
    void visit_derived_function_atom_pre(FunctionAtom *atom) {}
    bool visit_self_atom(SelfAtom *atom);
    bool visit_rule_atom(RuleAtom *atom);
    bool visit_boolean_atom(BooleanAtom *atom);
    bool visit_string_atom(StringAtom *atom);
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
