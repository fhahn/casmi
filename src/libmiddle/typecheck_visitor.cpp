#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver) {}

void TypecheckVisitor::visit_assert(UnaryNode *assert, Type val) {
  if (val != Type::BOOL) {
    driver_.error(assert->child_->location,
                  "type of expression should be `Bool` but was `" +type_to_str(val)+"`");
  }
}

void TypecheckVisitor::visit_update(UpdateNode *update, Type val) {
  Symbol *sym = driver_.current_symbol_table->get(update->sym_->name_);
  if (sym == nullptr) {
    driver_.error(update->sym_->location, "use of undefined function `"+update->sym_->name_+"`");
  }
  if (val != Type::UNDEF && sym->return_type_ != val) {
    driver_.error(update->location, "type `"+type_to_str(sym->return_type_)+"` of `"+update->sym_->name_+
                            "` does not match type `"+type_to_str(val)+"` of expression");
  }
  update->sym_->symbol = sym;
}

Type TypecheckVisitor::visit_expression(Expression *expr, Type left_val, Type right_val) {
  if (left_val != right_val) {
      driver_.error(expr->location, "type of expressions did not match");
  }
  switch (expr->op) {
    case Expression::Operation::ADD: return left_val;
    case Expression::Operation::EQ: return Type::BOOL;
    default: assert(0);
  }
}

Type TypecheckVisitor::visit_expression_single(Expression *expr, Type val) {
  // just pass to type of the expression up, nothing to check here
  UNUSED(expr);
  return val;
}

Type TypecheckVisitor::visit_function_atom(FunctionAtom *atom) {
  Symbol *sym = driver_.current_symbol_table->get(atom->func_->name_);
  atom->func_->symbol = sym;
  return sym->return_type_;
}
