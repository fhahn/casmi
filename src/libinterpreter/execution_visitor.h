#ifndef CASMI_LIBINTERPRETER_EXEC_VISITOR
#define CASMI_LIBINTERPRETER_EXEC_VISITOR

#include "libparse/ast.h"
#include "libparse/visitor.h"

class ExecutionVisitor : public AstVisitor {
  private:
    AstNode *root_;

  public:
    ExecutionVisitor(AstNode *root);
    void execute();
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
