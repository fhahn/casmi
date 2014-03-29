#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include "libparse/ast.h"
#include "libparse/visitor.h"
#include "libparse/symbols.h"

class ExecutionContext {
  private:
    SymbolTable *current_st_;

  public:
    ExecutionContext(SymbolTable *st);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
