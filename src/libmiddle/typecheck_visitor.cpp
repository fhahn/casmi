#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver) {}

void TypecheckVisitor::visit_function_def(FunctionDefNode *def,
                                          const std::vector<std::pair<Type, Type>>& initializers) {
  for (size_t i = 0; i < initializers.size(); i++) {
    const std::pair<Type, Type>& p = initializers[i];

    if (!def->sym->arguments_ && p.first == Type::UNDEF && p.second != def->sym->return_type_) {
      driver_.error(def->sym->intitializers_->at(i).second->location,
                  "type of initializer of function `" +def->sym->name()+
                  "` is `"+type_to_str(p.second)+"` but should be `"+
                  type_to_str(def->sym->return_type_)+"´");
    }
  }
}

void TypecheckVisitor::visit_ifthenelse(IfThenElseNode *node, Type cond) {
  if (cond != Type::BOOLEAN) {
    driver_.error(node->condition_->location,
                  "type of expression should be `Bool` but was `" +type_to_str(cond)+"`");
  }
}

void TypecheckVisitor::visit_assert(UnaryNode *assert, Type val) {
  if (val != Type::BOOLEAN) {
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

void TypecheckVisitor::visit_call_pre(CallNode *call) {
  if (driver_.rules_map_.count(call->rule_name) == 1) {
    call->rule = driver_.rules_map_[call->rule_name];
  } else {
    driver_.error(call->location, "no rule with name `"+call->rule_name+"` found");
  }
}

void TypecheckVisitor::visit_call_pre(CallNode *call, Type expr) {
  if (expr != Type::RULEREF) {
    driver_.error(call->ruleref->location, "Indirect target must be a `Ruleref` but was `"+
                                            type_to_str(expr)+"`");
  }
}


void TypecheckVisitor::visit_call(CallNode *call, std::vector<Type> argument_results) {
  size_t args_defined = (call->rule->arguments) ? call->rule->arguments->size() : 0;
  size_t args_provided = argument_results.size();
  if (args_defined != args_provided) {
    driver_.error(call->location, "rule `"+call->rule_name+"` expects "
                                  +std::to_string(args_defined)+" arguments but "+
                                  std::to_string(args_provided)+" where provided");
  } else {
    for (size_t i=0; i < args_defined; i++) {
      if (call->rule->arguments->at(i) != argument_results[i]) {
        driver_.error(call->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of rule `"+ call->rule_name+
                      "` must be `"+type_to_str(call->rule->arguments->at(i))+"` but was `"+
                      type_to_str(argument_results[i])+"`");
      }
    }
  }
}

void TypecheckVisitor::check_numeric_operator(const yy::location& loc, 
                                              const Type type,
                                              const Expression::Operation op) {
  if (type != Type::INT && type != Type::FLOAT && type != Type::UNDEF) {
    driver_.error(loc,
                  "operands of operator `"+operator_to_str(op)+
                  "` must be `Int` or `Float` but were `"+
                  type_to_str(type)+"`");
  }
}

Type TypecheckVisitor::visit_expression(Expression *expr, Type left_val, Type right_val) {
  if (left_val != right_val && left_val != Type::UNDEF && right_val != Type::UNDEF) {
      driver_.error(expr->location, "type of expressions did not match");
  }
  switch (expr->op) {
    case Expression::Operation::ADD:
    case Expression::Operation::SUB:
    case Expression::Operation::MUL:
    case Expression::Operation::DIV:
    case Expression::Operation::MOD:
    case Expression::Operation::RAT_DIV:
      check_numeric_operator(expr->location, left_val, expr->op);
      return left_val;

    case Expression::Operation::EQ:
    case Expression::Operation::NEQ:
      return Type::BOOLEAN;

    case Expression::Operation::LESSER:
    case Expression::Operation::GREATER:
    case Expression::Operation::LESSEREQ:
    case Expression::Operation::GREATEREQ:
      check_numeric_operator(expr->location, left_val, expr->op);
      return Type::BOOLEAN;
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
    driver_.error(atom->location, "no rule with name `"+atom->name+"` found");
  }
  return Type::RULEREF;
}
