#include "libsyntax/ast_dump_visitor.h"

AstDumpVisitor::AstDumpVisitor() { }

std::string AstDumpVisitor::get_dump() {
  
  return std::string("digraph \"main\" {\n") + dump_stream_.str() + std::string("}");
}

void AstDumpVisitor::add_node(uint64_t key, const std::string& name) {
  dump_stream_ << "    " << key << " [label=\"" << name << "\"];" << std::endl;
}

void AstDumpVisitor::dump_symbol_usage(const SymbolUsage* sym) {
  add_node((uint64_t) sym, std::string("SymbolUsage "+sym->name_));
}

void AstDumpVisitor::dump_link(uint64_t from, uint64_t to) {
  dump_stream_ << "    " << from << " -> " << to << ";" << std::endl;
}

void AstDumpVisitor::dump_link(AstNode *from, AstNode *to) {
  dump_link((uint64_t) from, (uint64_t) to);
}

void AstDumpVisitor::visit_body_elements(AstListNode *body_elements) {
  add_node((uint64_t) body_elements, "Body Elements");

  for (AstNode *s : body_elements->nodes) {
    dump_link((uint64_t) body_elements, (uint64_t) s);
  }
}

void AstDumpVisitor::visit_rule(RuleNode *rule) {
  add_node((uint64_t) rule, "Rule "+rule->name);
  dump_link((uint64_t) rule, (uint64_t) rule->child_);
}

void AstDumpVisitor::visit_statements(AstListNode *stmts) {
  add_node((uint64_t) stmts, "Statements");

  for (AstNode *s : stmts->nodes) {
    dump_link((uint64_t) stmts, (uint64_t) s);
  }
}

void AstDumpVisitor::visit_statement(AstNode *stmt) {
  add_node((uint64_t) stmt, "Statement");
  //dump_link((uint64_t) rule, (uint64_t) rule->child_);
}

void AstDumpVisitor::visit_parblock(UnaryNode *parblock) {
  add_node((uint64_t) parblock, "Parblock");
  dump_link(parblock, parblock->child_);
}

bool AstDumpVisitor::visit_update(UpdateNode *update, bool) {
  add_node((uint64_t) update, "Update");
  dump_symbol_usage(update->sym_);

  dump_link((uint64_t) update, (uint64_t) update->sym_);
  dump_link((uint64_t) update, (uint64_t) update->expr_);
  return true;
}

bool AstDumpVisitor::visit_expression(Expression *expr, bool, bool) {
  add_node((uint64_t) expr, "Expression");
  if (expr->left_ != nullptr) {
    dump_link((uint64_t) expr, (uint64_t) expr->left_);
  }
  if (expr->right_ != nullptr) {
    dump_link((uint64_t) expr, (uint64_t) expr->right_);
  }

  return true;
}

bool AstDumpVisitor::visit_expression_single(Expression *expr, bool) {
  visit_expression(expr, true, true);
  return true;
}

bool AstDumpVisitor::visit_int_atom(IntAtom *atom) {
  add_node((uint64_t) atom, std::string("IntAtom: ")+std::to_string(atom->val_));
  return true;
}
