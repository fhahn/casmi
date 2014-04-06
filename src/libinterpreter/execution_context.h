#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include "libsyntax/symbols.h"

#include "librt/pp_hashmap.h"
#include "librt/pp_casm_updateset_linkedhash.h"

#define UPDATESET_SIZE 65535


CASM_UPDATE_TYPE(4);

class ExecutionContext {
  private:
    SymbolTable *current_st_;
    pp_mem updateset_data_;

  public:
    casm_updateset updateset;
    pp_mem pp_stack;
    uint64_t pseudostate;

    ExecutionContext(SymbolTable *st);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
