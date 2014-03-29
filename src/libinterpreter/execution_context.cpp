#include "libinterpreter/execution_context.h"


ExecutionContext::ExecutionContext(SymbolTable *st) : current_st_(st) {}
