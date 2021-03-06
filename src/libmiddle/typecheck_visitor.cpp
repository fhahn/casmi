#include "libmiddle/function_cycle_visitor.h"
#include "libmiddle/typecheck_visitor.h"



TypecheckVisitor::TypecheckVisitor(Driver& driver) : driver_(driver), rule_binding_types(), rule_binding_offsets(), forall_head(false) { }

void TypecheckVisitor::check_type_valid(const yy::location& location, const Type& type) {
  if (type == TypeType::ENUM &&
      !driver_.function_table.get_enum(type.enum_name)) {
    driver_.error(location,
                  "unknown type "+type.enum_name+"");
  }
}

void TypecheckVisitor::visit_function_def(FunctionDefNode *def,
                                          const std::vector<std::pair<Type*, Type*>>& initializers) {

  check_type_valid(def->location, *def->sym->return_type_);

  // Check if argument types are valid
  for (Type* argument_t : def->sym->arguments_) {
      check_type_valid(def->location, *argument_t);
  }

  // check if initializer types match argument types
  for (size_t i = 0; i < initializers.size(); i++) {
    const std::pair<Type*, Type*>& p = initializers[i];

    // check type of initializer and type of function type
    if (!p.second->unify(def->sym->return_type_)) {
      driver_.error(def->sym->intitializers_->at(i).second->location,
                    "type of initializer of function `" +def->sym->name+
                    "` is "+p.second->to_str()+" but should be "+
                    def->sym->return_type_->to_str()+"");
    }

    // check arument types
    if (def->sym->arguments_.size() == 0) {
      if (def->sym->intitializers_->at(i).first) {
        driver_.error(def->sym->intitializers_->at(i).first->location,
                      "function `" +def->sym->name+
                       "` does not accept arguments but initializer provides some");
      }
    } else if (def->sym->arguments_.size() == 1) {
      if (def->sym->intitializers_->at(i).first && !p.first->unify(def->sym->arguments_[0])) {
        driver_.error(def->sym->intitializers_->at(i).first->location,
                      "type of initializer argument of function `" +def->sym->name+
                       "` is "+p.first->to_str()+" but should be "+
                       def->sym->arguments_[0]->to_str());
      } else if (!def->sym->intitializers_->at(i).first) {
        driver_.error(def->sym->intitializers_->at(i).second->location,
                      "function `" +def->sym->name+
                       "` needs one argument but initializer does not provide one");
      }
    } else {
      if (def->sym->intitializers_->at(i).first) {
        Type arg_tuple = Type(TypeType::TUPLE, def->sym->arguments_);
        if (!p.first->unify(&arg_tuple)) {
          driver_.error(def->sym->intitializers_->at(i).first->location,
                        "type of initializer arguments of function `" +def->sym->name+
                         "` is "+p.first->to_str()+" but should be "+
                         arg_tuple.to_str());
        }
      } else if (!def->sym->intitializers_->at(i).first) {
        driver_.error(def->sym->intitializers_->at(i).second->location,
                      "function `" +def->sym->name+
                       "` needs multiple arguments but initializer does not provide any");
      }
   
    }
  }

  InitCycleVisitor v;
  AstWalker<InitCycleVisitor, bool> walker(v);
  walker.walk_function_def(def);
  driver_.init_dependencies[def->sym->name] = walker.visitor.dependency_names;
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

  if (!def->sym->return_type_->unify(expr)) {
    driver_.error(def->location, "type of derived expression was `"+
                                 expr->to_str()+"` but should be `"+
                                 def->sym->return_type_->to_str()+"`");
  } else if (!def->sym->return_type_->is_complete()) {
    driver_.error(def->location, std::string("type of derived expression is ")+
                                 "unknown because type of expression is "+expr->to_str());
  }

}

void TypecheckVisitor::visit_rule(RuleNode *rule) {
  auto foo = new std::vector<Type*>();
  for (Type *arg : rule->arguments) {
    foo->push_back(arg);
  }
  rule_binding_types.push_back(foo);
  rule_binding_offsets.push_back(&rule->binding_offsets);

  for  (auto& pair : rule->dump_list) {
    for (const std::string& identifier : pair.second) {
      Function *sym = driver_.function_table.get_function(identifier);
      if (sym) {
        driver_.function_trace_map.emplace(sym->id, pair.first);
      } else {
        driver_.error(rule->location, std::string("names in dumplist must be function identifiers")+
                                     " but `"+identifier+"` is not a function");
      }
    } 
  }
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

void TypecheckVisitor::visit_update(UpdateNode *update, Type*, Type*) {
  if (update->func->symbol && update->func->symbol->is_static) {
    driver_.error(update->location, "cannot update static function `"+update->func->name+"`");
  }

  if (!update->func->type_.unify(&update->expr_->type_)) {
    driver_.error(update->location, "type `"+update->func->type_.get_most_general_type()->to_str()+"` of `"+
                                    update->func->name+"` does not match type `"+
                                    update->expr_->type_.get_most_general_type()->to_str()+"` of expression");
  }

  if (update->func->symbol_type == FunctionAtom::SymbolType::PARAMETER) {
    driver_.error(update->location, "cannot update `"+update->func->name+
                                    "` because it is a parameter, not a function");
  }

  update->type_ = update->func->type_;

  if (update->func->symbol && driver_.function_trace_map.count(update->func->symbol->id) > 0) {
    update->node_type_ = NodeType::UPDATE_DUMPS;
  }
  if (update->func->symbol && (update->func->symbol->subrange_return 
                               || update->func->symbol->subrange_arguments.size() > 0)) {
    update->node_type_ = NodeType::UPDATE_SUBRANGE;
  }
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
      if (!call->rule->arguments[i]->unify(argument_results[i])) {
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
}

void TypecheckVisitor::visit_diedie(DiedieNode *node, Type* msg) {
  if (node->msg && !node->msg->type_.unify(TypeType::STRING)) {
    driver_.error(node->msg->location,
                  "expression of `diedie` must be a String but was "+msg->to_str());
  }
}

void TypecheckVisitor::visit_let(LetNode *node, Type*) {
  if (node->type_ == TypeType::ENUM &&
      !driver_.function_table.get_enum(node->type_.enum_name)) {
    driver_.error(node->location,
                  "unknown type "+node->type_.enum_name+"");
  }

  if (!node->type_.unify(&node->expr->type_)) {
    driver_.error(node->location, "type of let conflicts with type of expression");
  }


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
  
  } else {
    if (node->to->symbol->is_static) {
        driver_.error(node->to->location, "cannot push into static function `"+
                                            node->to->symbol->name+"`");
    }
    if (!expr->unify(atom->subtypes[0])) {
      driver_.error(node->expr->location, 
                    "cannot push "+expr->get_most_general_type()->to_str()
                    +" into "+atom->to_str());
    }
  }
}

void TypecheckVisitor::visit_pop(PopNode *node) {
  if (!node->from_type.unify(&node->from->type_)) {
    driver_.error(node->from->location,
                  "`pop from` argument must be List(A) but was "+node->from->type_.to_str());
  }

  if (node->from->symbol_type != FunctionAtom::SymbolType::FUNCTION) {
    driver_.error(node->from->location, "can only pop from functions");
  } else {
    if (node->from->symbol->is_static) {
        driver_.error(node->from->location, "cannot pop from static function `"+node->from->symbol->name+"`");
    }
  }


  Function *sym = driver_.function_table.get_function(node->to->name);
  if (sym) {
    if (sym->is_static) {
      driver_.error(node->to->location, "cannot pop into static function `"+sym->name+"`");
    }

    if (node->to->arguments) {
      // TODO this should be doable!
      driver_.error(node->to->location, "cannot pop into function with arguments");
    }
    visit_function_atom(node->to, arguments, 0);
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

void TypecheckVisitor::visit_case(CaseNode *node, Type *expr, const std::vector<Type*>& case_labels) {
  if (node->case_list.size() - case_labels.size() > 1) {
   driver_.error(node->location,
                 "more than one default label in case");
  }

  for (size_t i=0; i < case_labels.size(); i++) {
    if (!expr->unify(case_labels[i])) {
      driver_.error(node->case_list[i].first->location,
                    "type of case expression ("+expr->get_most_general_type()->to_str()+") and label ("+
                    case_labels[i]->get_most_general_type()->to_str()+") do not match");
    }
  }
}

void TypecheckVisitor::check_numeric_operator(const yy::location& loc, 
                                            Type* type,
                                            const ExpressionOperation op) {
  if (*type == TypeType::UNKNOWN) {
    type->constraints.push_back(new Type(TypeType::INT));
    if (op != ExpressionOperation::MOD || op == ExpressionOperation::RAT_DIV) {
      type->constraints.push_back(new Type(TypeType::RATIONAL));
      type->constraints.push_back(new Type(TypeType::FLOAT));
    }
  } else {
    if (op == ExpressionOperation::MOD || op == ExpressionOperation::RAT_DIV) {
      if (*type != TypeType::INT) {
      driver_.error(loc,
                    "operands of operator `"+operator_to_str(op)+
                    "` must be Int but were "+type->to_str());
      }
     
    } else if (*type != TypeType::INT && *type != TypeType::FLOAT && *type != TypeType::RATIONAL) {
      driver_.error(loc,
                    "operands of operator `"+operator_to_str(op)+
                    "` must be Int, Float or Rational but were "+
                    type->to_str());
    }
  }
}


Type* TypecheckVisitor::visit_expression(Expression *expr, Type*, Type*) {
  if (expr->left_ && expr->right_ && !expr->left_->type_.unify(&expr->right_->type_)) {
      driver_.error(expr->location, "type of expressions did not match: "+
                                     expr->left_->type_.get_most_general_type()->to_str()+" != "+
                                     expr->right_->type_.get_most_general_type()->to_str());
  }

  switch (expr->op) {
    case ExpressionOperation::ADD:
    case ExpressionOperation::SUB:
    case ExpressionOperation::MUL:
    case ExpressionOperation::DIV:
    case ExpressionOperation::MOD:
      check_numeric_operator(expr->location, &expr->left_->type_, expr->op);
      expr->type_.unify(&expr->left_->type_);
      break;
    case ExpressionOperation::RAT_DIV:
      check_numeric_operator(expr->location, &expr->left_->type_, expr->op);
      expr->type_.unify(Type(TypeType::RATIONAL));
      break;

    case ExpressionOperation::EQ:
    case ExpressionOperation::NEQ:
      expr->type_.unify(Type(TypeType::BOOLEAN));
      break;

    case ExpressionOperation::LESSER:
    case ExpressionOperation::GREATER:
    case ExpressionOperation::LESSEREQ:
    case ExpressionOperation::GREATEREQ:
      check_numeric_operator(expr->location, &expr->left_->type_, expr->op);
      expr->type_.unify(Type(TypeType::BOOLEAN));
      break;

    case ExpressionOperation::OR:
    case ExpressionOperation::XOR:
    case ExpressionOperation::AND:
      if (!expr->left_->type_.unify(new Type(TypeType::BOOLEAN))) {
        driver_.error(expr->location,
                  "operands of operator `"+operator_to_str(expr->op)+
                  "` must be Boolean but are "+expr->left_->type_.to_str());
      }
      expr->type_.unify(Type(TypeType::BOOLEAN));
      break;
    default: FAILURE();
  }

  return &expr->type_;
}

Type* TypecheckVisitor::visit_expression_single(Expression *expr, Type*) {
  switch (expr->op) {
    case ExpressionOperation::NOT:
      if (!expr->left_->type_.unify(new Type(TypeType::BOOLEAN))) {
        driver_.error(expr->location,
                  "operand of `not` must be Boolean but is "+expr->left_->type_.to_str());
      }
      expr->type_.unify(Type(TypeType::BOOLEAN));
      return &expr->type_;
      break;
    default: FAILURE();
  }

}

Type* TypecheckVisitor::visit_function_atom(FunctionAtom *atom, Type* arguments[],
                                            uint16_t num_arguments) {
  Symbol *sym = driver_.function_table.get(atom->name);
  if (sym && sym->type == Symbol::SymbolType::ENUM) {
    atom->symbol_type = FunctionAtom::SymbolType::ENUM;
    atom->enum_ = reinterpret_cast<Enum*>(sym);
    if (!forall_head && atom->enum_->name == atom->name) {
      driver_.error(atom->location, "`"+atom->name+"` is an enum, not a member of an enum");
    }
    atom->type_.unify(new Type(TypeType::ENUM, sym->name));
    return &atom->type_;
  }

  if (!sym) {
    // check if a rule parameter with this name was defined
    if (rule_binding_offsets.size() > 0){
      auto current_rule_binding_offsets = rule_binding_offsets.back();
      auto current_rule_binding_types = rule_binding_types.back();

      if (current_rule_binding_offsets->count(atom->name) &&
          !atom->arguments) {
        atom->symbol_type = FunctionAtom::SymbolType::PARAMETER;
        atom->offset = current_rule_binding_offsets->at(atom->name);
        Type* binding_type = current_rule_binding_types->at(atom->offset);
        atom->type_.unify(binding_type);
        return &atom->type_;
      }
    }

    driver_.error(atom->location, "use of undefined function `"+atom->name+"`");
    atom->type_ = Type(TypeType::INVALID);
    return &atom->type_;
  }

  Function *func = reinterpret_cast<Function*>(sym);
  atom->symbol = func;
  if (atom->symbol->type == Symbol::SymbolType::FUNCTION) {
    atom->symbol_type = FunctionAtom::SymbolType::FUNCTION;
    if (func->subrange_arguments.size() > 0) {
      atom->node_type_ = NodeType::FUNCTION_ATOM_SUBRANGE;
    }
  } else{
    atom->symbol_type = FunctionAtom::SymbolType::DERIVED;
  }

  // check for function definitions with arguments
  if(atom->symbol->arguments_.size() != num_arguments) {
    driver_.error(atom->location,
                  "number of provided arguments does not match definition of `"+
                  atom->name+"`");
  } else {
    for (size_t i=0; i < atom->symbol->arguments_.size(); i++) {

      Type *argument_t = atom->symbol->arguments_[i];
 
      if (!arguments[i]->unify(argument_t)) {
        driver_.error(atom->arguments->at(i)->location,
                      "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                      "` is "+arguments[i]->to_str()+" but should be "+
                      argument_t->to_str());
      }
    }
  }

  // check for function definitions without arguments
  if (atom->symbol->arguments_.size() == 0 && num_arguments > 0 ) {
    driver_.error(atom->location, "number of provided arguments does not match definition of `"+atom->name+"`");
  }

  atom->type_.unify(func->return_type_);
  return &atom->type_;
}

Type* TypecheckVisitor::visit_builtin_atom(BuiltinAtom *atom,
                                           Type* arguments[],
                                           uint16_t num_arguments) {
  if(atom->types.size() != num_arguments) {
    driver_.error(atom->location,
                  "number of provided arguments does not match definition of `"+
                  atom->name+"`");
  } else {
    for (size_t i=0; i < atom->types.size(); i++) {

     Type *argument_t = atom->types[i];
 
      if (!arguments[i]->unify(argument_t)) {
        driver_.error(atom->arguments->at(i)->location,
                      "type of "+std::to_string(i+1)+" argument of `"+atom->name+
                      "` is "+arguments[i]->to_str()+" but should be "+
                      argument_t->to_str());
      }
    }
  }

  if (atom->name == "nth") {
    if (*atom->types[0] == TypeType::TUPLE_OR_LIST && atom->types[0]->subtypes.size() > 0 && atom->types[0]->subtypes[0]->t != TypeType::UNKNOWN) {
      Type first = *atom->types[0]->subtypes[0];
      bool all_equal = true;
      for (size_t i=1; i < atom->types[0]->subtypes.size(); i++) {
        if (first != *atom->types[0]->subtypes[i]) {
          all_equal = false;
          break;
        }
      }
      if (all_equal) {
        atom->types[0]->t = TypeType::LIST;
        atom->types[0]->subtypes = {new Type(first)};
      } 
    }

    if (*atom->types[0] == TypeType::TUPLE) {
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
        if ((size_t) ind < (atom->types[0]->subtypes.size()+1)) {
          atom->type_.unify(atom->types[0]->subtypes[ind-1]);
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
    } else {
      atom->type_.unify(atom->types[0]->subtypes[0]);
      arguments[0]->unify(&atom->type_);
      atom->type_.unify(atom->return_type);
    }
  } else {
    // TODO use type_ as return_type_ for builtins
    atom->type_.unify(atom->return_type);
  }
  return &atom->type_;
}

void TypecheckVisitor::visit_derived_function_atom_pre(FunctionAtom *atom,
                                                       Type* arguments[],
                                                       uint16_t num_arguments) {
  size_t args_defined = atom->symbol->arguments_.size();
  size_t args_provided = num_arguments;
  if (args_defined != args_provided) {
    driver_.error(atom->location, " expects "
                                  +std::to_string(args_defined)+" arguments but "+
                                  std::to_string(args_provided)+" where provided");
  } else {
    for (size_t i=0; i < args_defined; i++) {
      if (!arguments[i]->unify(atom->symbol->arguments_.at(i))) {
        driver_.error(atom->arguments->at(i)->location,
                      "argument "+std::to_string(i+1)+" of must be `"+atom->symbol->arguments_.at(i)->to_str()+"` but was `"+
                      arguments[i]->to_str()+"`");
      }
    }
  }
  rule_binding_types.push_back(new std::vector<Type*>(atom->symbol->arguments_));
  rule_binding_offsets.push_back(&atom->symbol->binding_offsets);
}

Type* TypecheckVisitor::visit_derived_function_atom(FunctionAtom*,
                                                    Type* expr) {
  delete rule_binding_types.back();
  rule_binding_types.pop_back();
  rule_binding_offsets.pop_back();
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
  atom->type_.subtypes = vals;

  if (vals.size() > 0) {
    Type first = *(vals[0]);
    bool all_known = first.is_complete();
    bool all_equal = true;
    for (size_t i=1; i < vals.size(); i++) {
      all_known = all_known && vals[i]->is_complete();
      if (first != *vals[i]) {
        all_equal = false;
        break;
      }
    }
    if (!all_equal && all_known) {
      atom->type_.t = TypeType::TUPLE;
    } 
  }

  return &atom->type_;
}

template <>
void AstWalker<TypecheckVisitor, Type*>::walk_forall(ForallNode *node) {

  visitor.forall_head = true;
  walk_expression_base(node->in_expr);
  visitor.forall_head = false;

  Type list_t = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));

  if (node->in_expr->type_ == TypeType::INT || node->in_expr->type_ == TypeType::ENUM) {
    node->type_.unify(&node->in_expr->type_);
  } else if (node->in_expr->type_.unify(&list_t)) {
    node->type_.unify(node->in_expr->type_.subtypes[0]);
  } else {
    visitor.driver_.error(node->location, "expression must be a List, an Int or enum, but is "
                                  +node->in_expr->type_.to_str());
  }

  auto current_rule_binding_types = visitor.rule_binding_types.back();
  auto current_rule_binding_offsets = visitor.rule_binding_offsets.back();

  current_rule_binding_offsets->insert(
      std::pair<std::string, size_t>(node->identifier,
                                     current_rule_binding_types->size())
  );
  current_rule_binding_types->push_back(&node->type_);

  walk_statement(node->statement);

  if (!node->type_.is_complete()) {
    visitor.driver_.error(node->location, "type inference for `"+node->identifier+"` failed");
  }

  visitor.rule_binding_types.back()->pop_back();
  visitor.rule_binding_offsets.back()->erase(node->identifier);
}

template <>
void AstWalker<TypecheckVisitor, Type*>::walk_call(CallNode *call) {
  // basically the same as in AstWalker, but we do not walk the rule here as
  // this could lead to an endless recursion
  if (call->ruleref == nullptr) {
    visitor.visit_call_pre(call);
  } else {
    Type *v = walk_expression_base(call->ruleref);
    visitor.visit_call_pre(call, v);
  }

  // we must evaluate all arguments, to set correct offset for bindings
  std::vector<Type*> argument_results;
  if (call->arguments != nullptr) {
    for (ExpressionBase *e: *call->arguments) {
      argument_results.push_back(walk_expression_base(e));
    }
  }
  if (call->rule != nullptr) {
    visitor.visit_call(call, argument_results);
    //walk_rule(call->rule);
    visitor.visit_call_post(call);
  } else {
    DEBUG("rule not set!");
  }
}

