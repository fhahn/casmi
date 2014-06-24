#ifndef CASMI_LIBPARSE_INIT_CYCLE_VISITOR
#define CASMI_LIBPARSE_INIT_CYCLE_VISITOR

#include <utility>
#include <set>

#include "libsyntax/visitor.h"

class InitCycleVisitor: public BaseVisitor<bool> {
  public:
    std::set<std::string> dependency_names;
    bool arguments[10];
    uint32_t num_arguments;

    bool visit_function_atom(FunctionAtom *atom, bool[], uint16_t);
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
