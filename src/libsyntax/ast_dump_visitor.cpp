#include "libsyntax/ast_dump_visitor.h"

AstDumpVisitor::AstDumpVisitor() { }

std::string AstDumpVisitor::get_dump() {
  
  return std::string("digraph \"main\" {\n") + dump_stream_.str() + std::string("}");
}

void AstDumpVisitor::add_node(uint64_t key, const std::string& name) {
  dump_stream_ << "    " << key << " [label=\"" << name << "\"];" << std::endl;
}

void AstDumpVisitor::visit_body_elements(AstListNode *body_elements) {
  add_node((uint64_t) body_elements, "Body Elements");

  for (AstNode *s : body_elements->nodes) {
    dump_stream_ << "    " << (uint64_t) body_elements << " -> " << (uint64_t) s << ";" << std::endl;
  }
}

bool AstDumpVisitor::visit_update(UpdateNode *update, bool) {
  return true;
}

bool AstDumpVisitor::visit_expression(Expression *expr, bool, bool) {
  return true;
}

bool AstDumpVisitor::visit_expression_single(Expression *expr, bool) {
  return true;
}
