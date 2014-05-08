#ifndef CASMI_LIBPARSE_INIT_CYCLE_VISITOR
#define CASMI_LIBPARSE_INIT_CYCLE_VISITOR

#include <utility>
#include <set>

#include "libsyntax/visitor.h"

class InitCycleVisitor: public BaseVisitor<bool> {
  public:
    std::set<std::string> dependency_names;

    bool visit_function_atom(FunctionAtom *atom,
                             const std::vector<bool> &expr_results);
};

#endif //CASMI_LIBINTERPRETER_EXEC_VISITOR
