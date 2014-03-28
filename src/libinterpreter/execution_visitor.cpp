#include "libinterpreter/execution_visitor.h"


ExecutionVisitor::ExecutionVisitor(AstNode *root) : root_(root) {}

void ExecutionVisitor::execute() {
  walk_specification(root_);
}
