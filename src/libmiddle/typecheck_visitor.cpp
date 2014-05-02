#include "libmiddle/typecheck_visitor.h"


TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver), rule_binding_types(), rule_binding_offsets() {}

void TypecheckVisitor::visit_function_def(FunctionDefNode *def,
                                          const std::vector<std::pair<Type*, Type*>>& initializers) {
  for (size_t i = 0; i < initializers.size(); i++) {
    const std::pair<Type*, Type*>& p = initializers[i];

    if (def->sym->arguments_.size() == 0 && *p.first == TypeType::UNDEF && *p.second != def->sym->return_type_) {
      driver_.error(def->sym->intitializers_->at(i).second->location,
                  "type of initializer of function `" +def->sym->name()+
                  "` is `"+p.second->to_str()+"` but should be `"+
                  def->sym->return_type_->to_str()+"´");
    }
  }
}

void TypecheckVisitor::visit_derived_def_pre(FunctionDefNode *def) {
  auto foo = new std::vector<Type*>();
  for (Type *arg : def->sym->arguments_) {
    foo->push_back(arg);
  }
  rule_binding_types.push_back(foo);
  rule_binding_offsets.push_back(&def->sym->binding_offsets);
}

void TypecheckVisitor::visit_derived_def(FunctionDefNode *def, Type* expr) {
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();

  if (*def->sym->return_type_ == TypeType::UNKNOWN) {
    if (*expr == TypeType::UNKNOWN) {
      driver_.error(def->location, std::string("type of derived expression is ")+
                                   "unknown because type of expression is `undef`");
   
    }
    // TODO unify
    def->sym->return_type_ = expr;
  } else if (*def->sym->return_type_ != *expr && *expr != TypeType::UNDEF) {
    driver_.error(def->location, "type of derived expression was `"+
                                 expr->to_str()+"` but should be `"+
                                 def->sym->return_type_->to_str()+"`");
  }
}

void TypecheckVisitor::visit_rule(RuleNode *rule) {
  auto foo = new std::vector<Type*>();
  for (Type *arg : rule->arguments) {
    foo->push_back(arg);
  }
  rule_binding_types.push_back(foo);
  rule_binding_offsets.push_back(&rule->binding_offsets);
}

void TypecheckVisitor::visit_ifthenelse(IfThenElseNode *node, Type* cond) {
  if (*cond != TypeType::BOOLEAN) {
    driver_.error(node->condition_->location,
                  "type of expression should be `Bool` but was `" +cond->to_str()+"`");
  }
}

void TypecheckVisitor::visit_assert(UnaryNode *assert, Type* val) {
  if (*val != TypeType::BOOLEAN) {
    driver_.error(assert->child_->location,
                  "type of expression should be `Bool` but was `" +val->to_str()+"`");
  }
}

void TypecheckVisitor::visit_update(UpdateNode *update, Type* func_t, Type* expr_t) {
  // TODO unify func->type and expr->type
  DEBUG("UNIFY update "<<update->func->type_.to_str() << " "<<update->expr_->type_.to_str());
  //DEBUG("link "<<update->func->type_.unify_links_to_str());
  //DEBUG("link "<<update->expr_->type_.unify_links_to_str());
  if (!update->func->type_.unify(&update->expr_->type_)) {
    driver_.error(update->location, "type `"+func_t->to_str()+"` of `"+
                                    update->func->name+"` does not match type `"+
                                    update->expr_->type_.to_str()+"` of expression");
  }

  if (update->func->symbol_type == FunctionAtom::SymbolType::PARAMETER) {
    driver_.error(update->location, "cannot update `"+update->func->name+
                                    "` because it is a parameter, not a function");
  }

  update->type_ = update->func->type_;
  DEBUG("Type of update "<<update->type_.to_str() << " func: "<<update->func->type_.to_str() << " expr: "<<update->expr_->type_.to_str());
}

void TypecheckVisitor::visit_call_pre(CallNode *call) {
  if (driver_.rules_map_.count(call->rule_name) == 1) {
    call->rule = driver_.rules_map_[call->rule_name];
  } else {
    driver_.error(call->location, "no rule with name `"+call->rule_name+"` found");
  }
}

void TypecheckVisitor::visit_call_pre(CallNode *call, Type* expr) {
  if (*expr != TypeType::RULEREF) {
    driver_.error(call->ruleref->location, "Indirect target must be a `Ruleref` but was `"+
                                            expr->to_str()+"`");
  }
}


void TypecheckVisitor::visit_call(CallNode *call, std::vector<Type*>& argument_results) {
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
      if (*call->rule->arguments[i] != *argument_results[i]) {
        driver_.error(call->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of rule `"+ call->rule_name+
                      "` must be `"+call->rule->arguments[i]->to_str()+"` but was `"+
                      argument_results[i]->to_str()+"`");
      }
  }
}
}

void TypecheckVisitor::visit_call_post(CallNode *call) {
  UNUSED(call);
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();
}

void TypecheckVisitor::visit_let(LetNode *node, Type* v) {
  DEBUG("LET "<<node->identifier << "\tT1 "<<node->type_.to_str()<< " T2 "<<v->to_str() << " addr "<<&node->type_);
  if (!node->type_.unify(&node->expr->type_)) {
    driver_.error(node->location, "type of let conflicts with type of expression");
  }

  DEBUG("LET UNIFIED "<<node->type_.to_str());
  DEBUG(node->type_.unify_links_to_str());

  auto current_rule_binding_types = rule_binding_types.back();
  auto current_rule_binding_offsets = rule_binding_offsets.back();

  current_rule_binding_offsets->insert(
      std::pair<std::string, size_t>(node->identifier,
                                     current_rule_binding_types->size())
  );
  current_rule_binding_types->push_back(&node->type_);
}

void TypecheckVisitor::visit_let_post(LetNode *node) {
  DEBUG("type of let "<<&node->type_ << " "<<node->type_.to_str());
  if (!node->type_.is_complete()) {
    driver_.error(node->location, "type inference for `"+node->identifier+"` failed");
  }
  rule_binding_types.back()->pop_back();
  rule_binding_offsets.back()->erase(node->identifier);
}

void TypecheckVisitor::visit_push(PushNode *node, Type *expr, Type *atom) {
  if (node->to->symbol_type != FunctionAtom::SymbolType::FUNCTION) {
    driver_.error(node->to->location, 
                  "can only push into functions");
  
  }
  if (!expr->unify(atom->internal_type)) {
    driver_.error(node->expr->location, 
                  "cannot push "+expr->get_most_general_type()->to_str()+" into "+atom->to_str());
  }
}

void TypecheckVisitor::visit_pop(PopNode *node) {
  if (!node->from_type.unify(&node->from->type_)) {
    driver_.error(node->from->location,
                  "from argument must be List(Unknown) but was "+node->from->type_.to_str());
  }

  Function *sym = driver_.function_table.get(node->to->name);
  if (sym) {
    std::vector<Type*> func_arguments;
    if (node->to->arguments) {
      // TODO this should be doable!
      driver_.error(node->to->location, "cannot pop into function with arguments");
    }
    visit_function_atom(node->to, func_arguments);
    if (!node->type_.unify(&node->to->type_)) {
      driver_.error(node->from->location,
                    "cannot pop from "+node->from->type_.to_str()+" into "+node->to->type_.to_str());
    }
  } else {
    auto current_rule_binding_types = rule_binding_types.back();
    auto current_rule_binding_offsets = rule_binding_offsets.back();

    if (current_rule_binding_offsets->count(node->to->name) != 0) {
      driver_.error(node->to->location,
                    "can only pop into functions or new bindings");
    }
    current_rule_binding_offsets->insert(
        std::pair<std::string, size_t>(node->to->name,
                                       current_rule_binding_types->size())
    );
    current_rule_binding_types->push_back(&node->type_);
    node->to->symbol_type = FunctionAtom::SymbolType::PUSH_POP;
    node->to->offset = current_rule_binding_offsets->at(node->to->name);
  }
}

void TypecheckVisitor::check_numeric_operator(const yy::location& loc, 
                                            Type* type,
                                            const Expression::Operation op) {
  if (*type == TypeType::UNKNOWN) {
    DEBUG("Add numeric constraints");
    type->constraints.push_back(new Type(TypeType::INT));
    type->constraints.push_back(new Type(TypeType::FLOAT));
  } else if (*type != TypeType::INT && *type != TypeType::FLOAT && *type != TypeType::UNDEF) {
    driver_.error(loc,
                  "operands of operator `"+operator_to_str(op)+
                  "` must be `Int` or `Float` but were `"+
                  type->to_str()+"`");
  }
}

Type* TypecheckVisitor::visit_expression(Expression *expr, Type* left_val, Type* right_val) {
  DEBUG("EXPR T1 "<<expr->left_->type_.to_str() << " T2: "<<expr->right_->type_.to_str());
  if (!expr->left_->type_.unify(&expr->right_->type_)) {
      driver_.error(expr->location, "type of expressions did not match: "+
                                     expr->left_->type_.to_str()+" != "+
                                     expr->right_->type_.to_str());
  }

  DEBUG("EXPR DONE T1 "<<expr->left_->type_.to_str() << " T2: "<<expr->right_->type_.to_str());
  switch (expr->op) {
    case Expression::Operation::ADD:
    case Expression::Operation::SUB:
    case Expression::Operation::MUL:
    case Expression::Operation::DIV:
    case Expression::Operation::MOD:
    case Expression::Operation::RAT_DIV:
      check_numeric_operator(expr->location, left_val, expr->op);
      // TODO Check unifying
      expr->type_.unify(&expr->left_->type_);
      break;

    case Expression::Operation::EQ:
    case Expression::Operation::NEQ:
      expr->type_.unify(Type(TypeType::BOOLEAN));
      break;

    case Expression::Operation::LESSER:
    case Expression::Operation::GREATER:
    case Expression::Operation::LESSEREQ:
    case Expression::Operation::GREATEREQ:
      check_numeric_operator(expr->location, left_val, expr->op);
      expr->type_.unify(Type(TypeType::BOOLEAN));
      break;
    default: assert(0);
  }

  return &expr->type_;
}

Type* TypecheckVisitor::visit_expression_single(Expression *expr, Type* val) {
  // just pass to type of the expression up, nothing to check here
  UNUSED(expr);
  return val;
}

Type* TypecheckVisitor::visit_function_atom(FunctionAtom *atom,
                                           const std::vector<Type*> &expr_results) {

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
        // TODO check unifying
        Type* binding_type = current_rule_binding_types->at(atom->offset);
        atom->type_.unify(binding_type);
        DEBUG("BINDING\t atom_t "<<atom->type_.to_str() << " binding_t "<<binding_type->to_str()) ;
        return &atom->type_;
      }
    }

    driver_.error(atom->location, "use of undefined function `"+atom->name+"`");
    atom->type_ = Type(TypeType::INVALID);
    return &atom->type_;
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

     Type *argument_t = atom->symbol->arguments_[i];
      DEBUG("UNIFY ARG11 "<< atom->symbol->name() << " "<<expr_results[i]->to_str() << " "<<argument_t);
      DEBUG(argument_t->unify_links_to_str());
 
      if (!expr_results[i]->unify(argument_t)) {
        driver_.error(atom->arguments->at(i)->location,
                      "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                      "` is "+expr_results[i]->to_str()+" but should be "+
                      argument_t->to_str());
      }
      DEBUG("UNIFY ARG done "<< atom->symbol->name() << " "<<expr_results[i]->to_str() <<"\n");
      expr_results[i]->unify_links_to_str();
    }
  }

  // check for function definitions without arguments
  if (atom->symbol->arguments_.size() == 0 && expr_results.size() > 0 ) {
    driver_.error(atom->location, "number of provided arguments does not match definition of `"+atom->name+"`");
  }

  // TODO check unifying
  atom->type_.unify(sym->return_type_);
  return &atom->type_;
}

Type* TypecheckVisitor::visit_builtin_atom(BuiltinAtom *atom,
                                           const std::vector<Type*> &expr_results) {
  if(atom->types.size() != expr_results.size()) {
    driver_.error(atom->location,
                  "number of provided arguments does not match definition of `"+
                  atom->name+"`");
  } else {
    for (size_t i=0; i < atom->types.size(); i++) {

     Type *argument_t = atom->types[i];
      DEBUG("UNIFY ARGS "<< atom->name << " "<<expr_results[i]->to_str() << " "<<argument_t);
      DEBUG(argument_t->unify_links_to_str());
 
      if (!expr_results[i]->unify(argument_t)) {
        driver_.error(atom->arguments->at(i)->location,
                      "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                      "` is "+expr_results[i]->to_str()+" but should be "+
                      argument_t->to_str());
      }
      DEBUG("UNIFY ARG done "<< atom->name << " "<<expr_results[i]->to_str() <<"\n");
      expr_results[i]->unify_links_to_str();
    }
  }

  if (atom->name == "nth") {
    if (*atom->types[0] == TypeType::TUPLE_OR_LIST && atom->types[0]->tuple_types.size() > 0) {
      Type first = *atom->types[0]->tuple_types[0];
      bool all_equal = true;
      for (size_t i=1; i < atom->types[0]->tuple_types.size(); i++) {
        if (first != *atom->types[0]->tuple_types[i]) {
          all_equal = false;
          break;
        }
      }
      if (all_equal) {
        atom->types[0]->t = TypeType::LIST;
        atom->types[0]->internal_type = new Type(first);
      }
    }

    if (*atom->types[0] == TypeType::LIST) {
      atom->type_.unify(atom->types[0]->internal_type);
    } else {
      ExpressionBase *ind_expr = atom->arguments->at(1);
      if (ind_expr->node_type_ == NodeType::INT_ATOM) {
        INT_T ind = reinterpret_cast<IntAtom*>(ind_expr)->val_;
        if (ind <= 0) {
          driver_.error(atom->arguments->at(1)->location,
                        "second argument of nth must be a positive (>0) Int constant for tuples");

          return &atom->type_;
        }

        // this is needed to handle stuff like:
        //          assert nth(undef, 2) = undef
        if (atom->types[0]->is_unknown() && atom->type_.is_unknown()) {
          return &atom->type_;
        }
        if ((size_t) ind < (atom->types[0]->tuple_types.size()+1)) {
          atom->type_.unify(atom->types[0]->tuple_types[ind-1]);
        } else {
          driver_.error(atom->arguments->at(1)->location,
                        "index out of bounds for tuple, currently tuple only has "+
                        std::to_string(atom->arguments->size())+" types");

        }
      } else {
        driver_.error(atom->arguments->at(1)->location,
                      "second argument of nth must be an Int constant for tuples but was `"+
                      type_to_str(ind_expr->node_type_)+"`");
      }
    }
  } else {
    // TODO check unifying
    // TODO use tpye_ as return_type_ for builtins
    atom->type_.unify(atom->return_type);
  }
    return &atom->type_;
}

void TypecheckVisitor::visit_derived_function_atom_pre(FunctionAtom *atom) {
  auto foo = new std::vector<Type*>();
  for (Type *arg : atom->symbol->arguments_) {
    foo->push_back(arg);
  }
  rule_binding_types.push_back(foo);
  rule_binding_offsets.push_back(&atom->symbol->binding_offsets);
}

Type* TypecheckVisitor::visit_derived_function_atom(FunctionAtom *atom,
                                                  const std::vector<Type*>& argument_results,
                                                  Type* expr) {
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
      if (!argument_results[i]->unify(atom->symbol->arguments_.at(i))) {
        driver_.error(atom->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of must be `"+atom->symbol->arguments_.at(i)->to_str()+"` but was `"+
                      argument_results[i]->to_str()+"`");
      }
    }
  }
  return expr;
}

Type* TypecheckVisitor::visit_rule_atom(RuleAtom *atom) {
  if (driver_.rules_map_.count(atom->name) == 1) {
    atom->rule = driver_.rules_map_[atom->name];
  } else {
    driver_.error(atom->location, "no rule with name `"+atom->name+"` found");
  }
  return &atom->type_;
}

Type* TypecheckVisitor::visit_list_atom(ListAtom *atom, std::vector<Type*> &vals) {
  atom->type_.t = TypeType::TUPLE_OR_LIST;
  atom->type_.tuple_types = vals;
  return &atom->type_;
}
