#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver) {}

void TypecheckVisitor::visit_assert(UnaryNode *assert, Type val) {
  if (val != Type::BOOL) {
    driver_.error(assert->child_->location,
                  "type of expression should be `Bool` but was `" +type_to_str(val)+"`");
  }
}

void TypecheckVisitor::visit_update(UpdateNode *update, Type func_t, Type expr_t) {
  if (expr_t != Type::UNDEF && func_t != expr_t) {
    driver_.error(update->location, "type `"+type_to_str(func_t)+"` of `"+
                                    update->func->name+"` does not match type `"+
                                    type_to_str(expr_t)+"` of expression");
  }
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

Type TypecheckVisitor::visit_function_atom(FunctionAtom *atom,
                                           const std::vector<Type> &expr_results) {
  Symbol *sym = driver_.current_symbol_table->get(atom->name);
  if (!sym) {
    driver_.error(atom->location, "use of undefined function `"+atom->name+"`");
    return Type::INVALID;
  }

  atom->symbol = sym;

  // check for function definitions with arguments
  if (atom->symbol->arguments_) {
    if(atom->symbol->arguments_->size() != expr_results.size()) {
      driver_.error(atom->location,
                    "number of provided arguments does not match definition of `"+
                    atom->name+"`");
    } else {
      for (size_t i=0; i < atom->symbol->arguments_->size(); i++) {
        if (atom->symbol->arguments_->at(i) != expr_results[i]) {
          driver_.error(atom->arguments->at(i)->location,
                        "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                        "` is "+type_to_str(expr_results[i])+" but should be "+
                        type_to_str(atom->symbol->arguments_->at(i)));
        }
      }
    }
  }

  // check for function definitions without arguments
  if (!atom->symbol->arguments_ && expr_results.size() > 0 ) {
    driver_.error(atom->location, "number of provided arguments does not match definition of `"+atom->name+"`");
  }

  return sym->return_type_;
}

Type TypecheckVisitor::visit_rule_atom(RuleAtom *atom) {
  if (driver_.rules_map_.count(atom->name) == 1) {
    atom->rule = driver_.rules_map_[atom->name];
  } else {
    driver_.error(atom->location, "No rule with name `"+atom->name+"` found");
  }
  return Type::RULEREF;
}
