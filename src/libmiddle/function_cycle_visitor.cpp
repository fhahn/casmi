#include "libmiddle/function_cycle_visitor.h"

bool InitCycleVisitor::visit_function_atom(FunctionAtom *atom,
                                               const std::vector<bool> &) {
  dependency_names.insert(atom->name);
  return true;
}
