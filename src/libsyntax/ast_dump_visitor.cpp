#include "libsyntax/ast_dump_visitor.h"

AstDumpVisitor::AstDumpVisitor() { }

std::string AstDumpVisitor::get_dump() {
  return std::string("digraph \"main\" {\n") + dump_stream_.str() + std::string("}");
}

void AstDumpVisitor::dump_node(uint64_t key, const std::string& name) {
  dump_stream_ << "    " << key << " [label=\"" << name << "\"];" << std::endl;
}

void AstDumpVisitor::dump_node(AstNode *n, const std::string& name) {
  dump_node((uint64_t) n, name);
}


void AstDumpVisitor::dump_link(uint64_t from, uint64_t to) {
  dump_stream_ << "    " << from << " -> " << to << ";" << std::endl;
}

void AstDumpVisitor::dump_link(AstNode *from, AstNode *to) {
  dump_link((uint64_t) from, (uint64_t) to);
}

void AstDumpVisitor::visit_body_elements(AstListNode *body_elements) {
  dump_node(body_elements, "Body Elements");

  for (AstNode *s : body_elements->nodes) {
    dump_link(body_elements, s);
  }
}

void AstDumpVisitor::visit_function_def(FunctionDefNode *def, const std::vector<std::pair<bool, bool>>&) {
  dump_node(def, "Function Definition: "+def->sym->to_str());
  throw "TODO: implement initializers";
}

void AstDumpVisitor::visit_init(AstNode *init) {
  dump_node(init, "Init");
}

void AstDumpVisitor::visit_rule(RuleNode *rule) {
  dump_node(rule, "Rule "+rule->name);
  dump_link(rule, rule->child_);
}

void AstDumpVisitor::visit_statements(AstListNode *stmts) {
  dump_node(stmts, "Statements");

  for (AstNode *s : stmts->nodes) {
    dump_link(stmts, s);
  }
}

void AstDumpVisitor::visit_statement(AstNode *stmt) {
  dump_node(stmt, "Statement");
  //dump_link((uint64_t) rule, (uint64_t) rule->child_);
}

void AstDumpVisitor::visit_ifthenelse(IfThenElseNode *node, bool) {
  dump_node(node, "IfThenElse");
  dump_link(node, node->condition_);
  dump_link(node, node->then_);
  if (node->else_) {
    dump_link(node, node->else_);
  }
}

bool AstDumpVisitor::visit_assert(UnaryNode *assert, bool) {
  dump_node(assert, "Assert");

  dump_link((uint64_t) assert, (uint64_t) assert->child_);
  return true;
}

void AstDumpVisitor::visit_parblock(UnaryNode *parblock) {
  dump_node(parblock, "Parblock");
  dump_link(parblock, parblock->child_);
}

bool AstDumpVisitor::visit_update(UpdateNode *update, bool, bool) {
  dump_node(update, "Update");

  dump_link((uint64_t) update, (uint64_t) update->func);
  dump_link(update, update->expr_);
  return true;
}

bool AstDumpVisitor::visit_call_pre(CallNode *call) {
  dump_node(call, "Call: "+call->rule_name);
  // TODO we need to populate the rule here!!!
  assert(0);
}

bool AstDumpVisitor::visit_call(CallNode *call, std::vector<bool>& argument_results) {
  UNUSED(call);
  UNUSED(argument_results);
}

bool AstDumpVisitor::visit_expression(Expression *expr, bool, bool) {
  dump_node(expr, "Expression:"+operator_to_str(expr->op));
  if (expr->left_ != nullptr) {
    dump_link(expr, expr->left_);
  }
  if (expr->right_ != nullptr) {
    dump_link(expr, expr->right_);
  }

  return true;
}

bool AstDumpVisitor::visit_expression_single(Expression *expr, bool) {
  visit_expression(expr, true, true);
  return true;
}

bool AstDumpVisitor::visit_int_atom(IntAtom *atom) {
  dump_node(atom, std::string("IntAtom: ")+std::to_string(atom->val_));
  return true;
}

bool AstDumpVisitor::visit_float_atom(FloatAtom *atom) {
  dump_node(atom, std::string("FloatAtom: ")+std::to_string(atom->val_));
  return true;
}

bool AstDumpVisitor::visit_function_atom(FunctionAtom *atom,
    const std::vector<bool> &expr_results) {
  dump_node(atom, std::string("FunctionAtom:"+atom->name));
  return true;
}

bool AstDumpVisitor::visit_self_atom(SelfAtom *atom) {
  dump_node(atom, std::string("SelfAtom"));
  return true;
}

bool AstDumpVisitor::visit_rule_atom(RuleAtom *atom) {
  dump_node(atom, std::string("RuleAtom: "+atom->name));
  return true;
}

bool AstDumpVisitor::visit_boolean_atom(BooleanAtom *atom) {
  dump_node(atom, std::string("BooleanAtom: "+atom->value));
  return true;
}
