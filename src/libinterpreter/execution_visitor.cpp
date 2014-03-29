#include "libinterpreter/execution_visitor.h"


ExecutionVisitor::ExecutionVisitor(AstNode *root, ExecutionContext& ctx) 
  : root_(root), context_(ctx) {}

void ExecutionVisitor::visit_update(UpdateNode *update) {
  std::cout << "HALLO\n";
}
