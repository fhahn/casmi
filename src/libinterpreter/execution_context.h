#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include "libsyntax/symbols.h"

class ExecutionContext {
  private:
    SymbolTable *current_st_;

  public:
    ExecutionContext(SymbolTable *st);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
