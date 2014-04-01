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

    void add_node(uint64_t key, const std::string& name);

  public:

    AstDumpVisitor();

    std::string get_dump();
    void visit_specification(AstNode *spec) {}
    void visit_body_elements(AstListNode *body_elements);
    void visit_rule(RuleNode *rule) {}
    void visit_statement(AstNode *stmt) {}
    void visit_parblock(UnaryNode *parblock) {}
    void visit_statements(AstListNode *stmts) {}
    bool visit_update(UpdateNode *update, bool);
    bool visit_expression(Expression *expr, bool, bool);
    bool visit_expression_single(Expression *expr, bool);
    bool visit_int_atom(IntAtom *atom) { return false; }
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
