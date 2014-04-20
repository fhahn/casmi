#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver), rule_binding_types(), rule_binding_offsets() {}

void TypecheckVisitor::visit_function_def(FunctionDefNode *def,
                                          const std::vector<std::pair<Type, Type>>& initializers) {
  for (size_t i = 0; i < initializers.size(); i++) {
    const std::pair<Type, Type>& p = initializers[i];

    if (def->sym->arguments_.size() == 0 && p.first == Type::UNDEF && p.second != def->sym->return_type_) {
      driver_.error(def->sym->intitializers_->at(i).second->location,
                  "type of initializer of function `" +def->sym->name()+
                  "` is `"+type_to_str(p.second)+"` but should be `"+
                  type_to_str(def->sym->return_type_)+"´");
    }
  }
}

void TypecheckVisitor::visit_derived_def_pre(FunctionDefNode *def) {
  rule_binding_types.push_back(&def->sym->arguments_);
  rule_binding_offsets.push_back(&def->sym->binding_offsets);
}

void TypecheckVisitor::visit_derived_def(FunctionDefNode *def, Type& expr) {
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();

  if (def->sym->return_type_ == Type::UNKNOWN) {
    if (expr == Type::UNDEF) {
      driver_.error(def->location, std::string("type of derived expression is ")+
                                   "unknown because type of expression is `undef`");
   
    }
    def->sym->return_type_ = expr;
  } else if (def->sym->return_type_ != expr && expr != Type::UNDEF) {
    driver_.error(def->location, "type of derived expression was `"+
                                 type_to_str(expr)+"` but should be `"+
                                 type_to_str(def->sym->return_type_)+"`");
  }
}

void TypecheckVisitor::visit_rule(RuleNode *rule) {
  rule_binding_types.push_back(&rule->arguments);
  rule_binding_offsets.push_back(&rule->binding_offsets);
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

  if (update->func->symbol_type == FunctionAtom::SymbolType::PARAMETER) {
    driver_.error(update->location, "cannot update `"+update->func->name+
                                    "` because it is a parameter, not a function");
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


void TypecheckVisitor::visit_call(CallNode *call, std::vector<Type>& argument_results) {
  // typecheck for indirect calls happens during execution
  if (call->ruleref) {
    return;
  }

  size_t args_defined = call->rule->arguments.size();
  size_t args_provided = argument_results.size();
  if (args_defined != args_provided) {
    driver_.error(call->location, "rule `"+call->rule_name+"` expects "
                                  +std::to_string(args_defined)+" arguments but "+
                                  std::to_string(args_provided)+" where provided");
  } else {
    for (size_t i=0; i < args_defined; i++) {
      if (call->rule->arguments[i] != argument_results[i]) {
        driver_.error(call->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of rule `"+ call->rule_name+
                      "` must be `"+type_to_str(call->rule->arguments[i])+"` but was `"+
                      type_to_str(argument_results[i])+"`");
      }
    }
  }
}

void TypecheckVisitor::visit_call_post(CallNode *call) {
  UNUSED(call);
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();
}

void TypecheckVisitor::visit_let(LetNode *node, Type& v) {
  if (node->type_ != Type::UNKNOWN && node->type_ != v) {
    driver_.error(node->location, "type of let conflicts with type of expression");
  }

  auto current_rule_binding_types = rule_binding_types.back();
  auto current_rule_binding_offsets = rule_binding_offsets.back();

  current_rule_binding_offsets->insert(
      std::pair<std::string, size_t>(node->identifier,
                                     current_rule_binding_types->size())
  );
  current_rule_binding_types->push_back(v);
}

void TypecheckVisitor::visit_let_post(LetNode *node) {
  rule_binding_types.back()->pop_back();
  rule_binding_offsets.back()->erase(node->identifier);
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
  Function *sym = driver_.function_table.get(atom->name);
  if (!sym) {
    // check if a rule parameter with this name was defined
    if (rule_binding_offsets.size() > 0){
      auto current_rule_binding_offsets = rule_binding_offsets.back();
      auto current_rule_binding_types = rule_binding_types.back();

      if (current_rule_binding_offsets->count(atom->name) &&
          !atom->arguments) {
        atom->symbol_type = FunctionAtom::SymbolType::PARAMETER;
        atom->offset = current_rule_binding_offsets->at(atom->name);
        Type t = current_rule_binding_types->at(atom->offset);
        return t;
      }
    }

    driver_.error(atom->location, "use of undefined function `"+atom->name+"`");
    return Type::INVALID;
  }

  atom->symbol = sym;
  if (atom->symbol->symbol_type == Function::SType::FUNCTION) {
    atom->symbol_type = FunctionAtom::SymbolType::FUNCTION;
  } else {
    atom->symbol_type = FunctionAtom::SymbolType::DERIVED;
  }

  // check for function definitions with arguments
  if(atom->symbol->arguments_.size() != expr_results.size()) {
    driver_.error(atom->location,
                  "number of provided arguments does not match definition of `"+
                  atom->name+"`");
  } else {
    for (size_t i=0; i < atom->symbol->arguments_.size(); i++) {
      if (atom->symbol->arguments_[i] != expr_results[i]) {
        driver_.error(atom->arguments->at(i)->location,
                      "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                      "` is "+type_to_str(expr_results[i])+" but should be "+
                      type_to_str(atom->symbol->arguments_[i]));
      }
    }
  }

  // check for function definitions without arguments
  if (atom->symbol->arguments_.size() == 0 && expr_results.size() > 0 ) {
    driver_.error(atom->location, "number of provided arguments does not match definition of `"+atom->name+"`");
  }

  return sym->return_type_;
}

void TypecheckVisitor::visit_derived_function_atom_pre(FunctionAtom *atom) {
  rule_binding_types.push_back(&atom->symbol->arguments_);
  rule_binding_offsets.push_back(&atom->symbol->binding_offsets);
}

Type TypecheckVisitor::visit_derived_function_atom(FunctionAtom *atom,
                                                  const std::vector<Type>& argument_results,
                                                  Type expr) {
  DEBUG("expr type "<<type_to_str(expr));
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();

  size_t args_defined = atom->symbol->arguments_.size();
  size_t args_provided = argument_results.size();
  if (args_defined != args_provided) {
    driver_.error(atom->location, " expects "
                                  +std::to_string(args_defined)+" arguments but "+
                                  std::to_string(args_provided)+" where provided");
  } else {
    for (size_t i=0; i < args_defined; i++) {
      if (atom->symbol->arguments_.at(i) != argument_results[i]) {
        driver_.error(atom->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of must be `"+type_to_str(atom->symbol->arguments_.at(i))+"` but was `"+
                      type_to_str(argument_results[i])+"`");
      }
    }
  }
  return expr;
}

Type TypecheckVisitor::visit_rule_atom(RuleAtom *atom) {
  if (driver_.rules_map_.count(atom->name) == 1) {
    atom->rule = driver_.rules_map_[atom->name];
  } else {
    driver_.error(atom->location, "no rule with name `"+atom->name+"` found");
  }
  return Type::RULEREF;
}
