#include "libmiddle/function_cycle_visitor.h"

bool InitCycleVisitor::visit_function_atom(FunctionAtom *atom, bool[], uint16_t) {
  dependency_names.insert(atom->name);
  return true;
}
