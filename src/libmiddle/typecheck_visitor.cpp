#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver) {}

void TypecheckVisitor::visit_update(UpdateNode *update, Type val) {
  Symbol *sym = driver_.current_symbol_table->get(update->sym_->name_);
  if (sym == nullptr) {
    driver_.error(update->sym_->location, "use of undefined function `"+update->sym_->name_+"`");
  }
  if (sym->return_type_ != val) {
    driver_.error(update->location, "type of `"+update->sym_->name_+
                            "` does not match type of expression");
  }
  update->sym_->symbol = sym;
}

Type TypecheckVisitor::visit_expression(Expression *expr, Type left_val, Type right_val) {
  if (left_val != right_val) {
      driver_.error(expr->location, "type of expressions did not match");
  }
  return left_val;
}

Type TypecheckVisitor::visit_expression_single(Expression *expr, Type val) {
  // just pass to type of the expression up, nothing to check here
  UNUSED(expr);
  return val;
}
