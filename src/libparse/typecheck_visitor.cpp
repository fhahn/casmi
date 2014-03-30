#include "libparse/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(casmi_driver& driver) : driver_(driver) {}

void TypecheckVisitor::visit_update(UpdateNode *update, Type val) {
  Type sym_type = driver_.current_symbol_table->get(update->sym_);
  if (sym_type == Type::INVALID) {
    driver_.error(update->sym_->location, "use of undefined function `"+update->sym_->name_+"`");
  }
  if (sym_type != val) {
    driver_.error(update->location, "type of `"+update->sym_->name_+
                            "` does not match type of expression");
  }
}

Type TypecheckVisitor::visit_expression(Expression *expr, Type left_val, Type right_val) {
  if (left_val != right_val) {
      driver_.error(expr->location, "type of expressions did not match");
  }
  return left_val;
}

Type TypecheckVisitor::visit_expression_single(Expression *expr, Type val) {
  return val;
}
