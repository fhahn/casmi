#include "macros.h"

#include "libinterpreter/execution_visitor.h"

/*
ExecutionVisitor::ExecutionVisitor() { }

std::string ExecutionVisitor::get_dump() {
  
  return std::string("digraph \"main\" {\n") + dump_stream_.str() + std::string("}");
}

void ExecutionVisitor::add_node(uint64_t key, const std::string& name) {
  dump_stream_ << "    " << key << " [label=\"" << name << "\"];" << std::endl;
}

void ExecutionVisitor::visit_body_elements(AstListNode *body_elements) {
  add_node((uint64_t) body_elements, "Body Elements");

  for (AstNode *s : body_elements->nodes) {
    dump_stream_ << "    " << (uint64_t) body_elements << " -> " << (uint64_t) s << ";" << std::endl;
  }
}

bool ExecutionVisitor::visit_update(UpdateNode *update, bool) {
  return true;
}

bool ExecutionVisitor::visit_expression(Expression *expr, bool, bool) {
  return true;
}

bool ExecutionVisitor::visit_expression_single(Expression *expr, bool) {
  return true;
}
*/
