#include <algorithm>
#include <cmath>
#include <assert.h>
#include <utility>

#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"
#include "libinterpreter/builtins.h"
#include "libinterpreter/operators.h"


DEFINE_CASM_UPDATESET_FORK_PAR
DEFINE_CASM_UPDATESET_FORK_SEQ


std::hash<Value> hasher;

void pack_values_in_array(const std::vector<Value> &value_list, uint64_t array[]) {
  for (size_t i=0; i < value_list.size(); i++) {
    array[i] = hasher(value_list[i]);
  }
}


ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, Driver& driver)
    : driver_(driver), context_(ctxt) {
  rule_bindings.push_back(&main_bindings);
}

void ExecutionVisitor::visit_assert(UnaryNode* assert, Value& val) {
  if (val.value.bval != true) {
    driver_.error(assert->location,
                  "Assertion failed");
    throw RuntimeException("Assertion failed");
  }
}

casm_update *ExecutionVisitor::add_update(const Value& val, size_t sym_id, const std::vector<Value> &arguments) {
  casm_update* up = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

  up->value = (void*) val.to_uint64_t();
  up->defined = (val.is_undef()) ? 0 : 1;
  up->func = sym_id;
  // TODO: Do we need line here?
  //up->line = (uint64_t) loc.lines;
  pack_values_in_array(arguments, up->args);

  up->num_args = arguments.size();

  auto& function_map = context_.functions[sym_id];
  if (function_map.second.count({up->args, up->num_args}) == 0) {
    function_map.second.emplace(ArgumentsKey{up->args, up->num_args}, Value());
  }
  Value& ref = function_map.second[{up->args, up->num_args}];
  casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                    (void*) &ref,
                                                    (void*) up);
  if (v != nullptr) {
    // Check if values match
    for (int i=0; i < up->num_args; i++) {
      if (up->args[i] != v->args[i]) {
        return up;
      }
    }
    throw RuntimeException("Conflict in updateset");
  }
  return up;
}

void ExecutionVisitor::visit_update(UpdateNode *update, Value &func_val, Value& expr_v) {
  UNUSED(func_val);

  try {
    casm_update *up = add_update(expr_v, update->func->symbol->id, value_list);
    up->line = (uint64_t) &update->location;
    value_list.clear();
    DEBUG("UPADTE "<<update->func->name<<" num args "<<up->num_args << " arg[0] "<<up->args[0]<< " val "<<expr_v.to_str());
  } catch (const RuntimeException& ex) {
    // TODO this is probably not the cleanest solutions
    driver_.error(update->location,
                  "update conflict in parallel block for function `"+update->func->name+"`");
    throw ex;
  }
}

void ExecutionVisitor::visit_call_pre(CallNode *call) { UNUSED(call); }

void ExecutionVisitor::visit_call_pre(CallNode *call, Value& expr) {
  if (expr.type != TypeType::UNDEF) {
    call->rule = expr.value.rule;
  } else {
    throw RuntimeException("Cannot call UNDEF");
  }
}

void ExecutionVisitor::visit_call(CallNode *call, std::vector<Value> &argument_results) {
  UNUSED(call);

  if (call->ruleref) {
    size_t args_defined = call->rule->arguments.size();
    size_t args_provided = argument_results.size();
    if (args_defined != args_provided) {
      driver_.error(call->location, "indirectly called rule `"+call->rule->name+
                    "` expects "+std::to_string(args_defined)+" arguments but "+
                    std::to_string(args_provided)+" where provided");
      throw RuntimeException("Invalid indirect call");
    } else {
      for (size_t i=0; i < args_defined; i++) {
        Type arg_t(argument_results[i].type);
        if (call->rule->arguments[i]->t == TypeType::LIST) {
          // TODO
          assert(0);
        } else if (call->rule->arguments[i]->t == TypeType::LIST) {
          // TODO
          assert(0);
        } else if (!call->rule->arguments[i]->unify(&arg_t) && !(argument_results[i].is_undef() && argument_results[i].type == TypeType::UNDEF)) {
          driver_.error(call->arguments->at(i)->location,
                        "argument "+std::to_string(i+1)+" of indirectly called rule `"+
                        call->rule->name+"` must be `"+
                        call->rule->arguments[i]->to_str()+"` but was `"+
                        Type(argument_results[i].type).to_str()+"`");
          throw RuntimeException("Invalid indirect call");
        }
      }
    }
  }
 
  rule_bindings.push_back(&argument_results);
}

void ExecutionVisitor::visit_call_post(CallNode *call) {
  UNUSED(call);
  rule_bindings.pop_back();
}

void ExecutionVisitor::visit_print(PrintNode *node, const std::vector<Value> &arguments) {
  if (node->filter.size() > 0 ) {
    if (context_.filter_enabled(node->filter)) {
      std::cout << node->filter << ": ";
    } else {
      return;
    }
  }

  for (const Value& v: arguments) {
    std::cout << v.to_str();
  }
  std::cout << std::endl;
}

void ExecutionVisitor::visit_let(LetNode *node, Value& v) {
  rule_bindings.back()->push_back(v);
}

void ExecutionVisitor::visit_let_post(LetNode *node) {
  rule_bindings.back()->pop_back();
}

void ExecutionVisitor::visit_push(PushNode *node, const Value& expr, const Value& atom) {
  Value to_res = builtins::cons(context_, expr, atom);

  try {
    casm_update *up = add_update(to_res, node->to->symbol->id, value_list);
    up->line = (uint64_t) &node->location;
    value_list.clear();
  } catch (const RuntimeException& ex) {
    // TODO this is probably not the cleanest solutions
    driver_.error(node->to->location,
                  "update conflict in parallel block for function `"+node->to->name+"`");
    throw ex;
  }
}

void ExecutionVisitor::visit_pop(PopNode *node, const Value& val) {
  Value to_res = builtins::peek(val);

  if (node->to->symbol_type == FunctionAtom::SymbolType::FUNCTION) {
    try {
      casm_update *up = add_update(to_res, node->to->symbol->id, value_list);
      up->line = (uint64_t) &node->location;
      value_list.clear();
    } catch (const RuntimeException& ex) {
      // TODO this is probably not the cleanest solutions
      driver_.error(node->to->location,
                    "update conflict in parallel block for function `"+node->to->name+"`");
      throw ex;
    }
  } else {
    rule_bindings.back()->push_back(to_res);
  } 

  Value from_res = builtins::tail(context_, val);
  try {
    casm_update *up = add_update(from_res, node->from->symbol->id, value_list);
    up->line = (uint64_t) &node->location;
    value_list.clear();
    DEBUG("POP");
  } catch (const RuntimeException& ex) {
    // TODO this is probably not the cleanest solutions
    driver_.error(node->location,
                  "update conflict in parallel block for function `"+node->from->name+"`");
    throw ex;
  }
  DEBUG("POPed "<<to_res.to_str() << " from "<<val.to_str() << " -> "<<from_res.to_str());
}

void ExecutionVisitor::visit_case(CaseNode *node, const Value& val) {
  
}

Value ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  DEBUG("left "<<left_val.to_str()<<" right "<<right_val.to_str());
  return operators::dispatch(expr->op, left_val, right_val);
}

Value ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  UNUSED(expr);
  return operators::dispatch(expr->op, val, val);
}

Value ExecutionVisitor::visit_function_atom(FunctionAtom *atom, std::vector<Value> &expr_results) {
  auto current_rule_bindings = rule_bindings.back();
  switch (atom->symbol_type) {
    case FunctionAtom::SymbolType::PARAMETER:
      return Value(current_rule_bindings->at(atom->offset));

    case FunctionAtom::SymbolType::FUNCTION: {
      size_t num_args = 1;
      if (expr_results.size() > 0) {
        num_args = expr_results.size();
      }

      uint64_t args[num_args];
      args[0] = 0;

      pack_values_in_array(expr_results, args);
      // TODO handle function access and function write differently
      value_list.swap(expr_results);

      Value v = Value(context_.get_function_value(atom->symbol, args));
      DEBUG("visit_atom "<<atom->symbol->name<<" "<<v.to_str() <<" size "<<value_list.size());
      return v;
    }
    case FunctionAtom::SymbolType::ENUM: {
      enum_value_t *val = atom->enum_->mapping[atom->name];
      Value v = Value(val);
      v.type = TypeType::ENUM;
      return v;
    }
    default: {
      DEBUG("visiting invalid symbol type of atom "<<atom->name<<" "<<atom->offset);
      assert(0);
    }
  }
}

Value ExecutionVisitor::visit_builtin_atom(BuiltinAtom *atom, std::vector<Value> &expr_results) {
  return builtins::dispatch(atom->id, context_, expr_results);
}

void ExecutionVisitor::visit_derived_function_atom_pre(FunctionAtom *atom, std::vector<Value>& arguments) {
  rule_bindings.push_back(&arguments);
}

Value ExecutionVisitor::visit_derived_function_atom(FunctionAtom *atom, Value& expr) {
  rule_bindings.pop_back();
  return expr;
}

Value ExecutionVisitor::visit_list_atom(ListAtom *atom, std::vector<Value> &vals) {
  BottomList *list = new BottomList(vals);
  // this could be faster if the list of expressions would be evaluated back to
  // front as well
  std::reverse(list->values.begin(), list->values.end());
  //context_.temp_lists.push_back(list);
  return Value(atom->type_, list);
}

std::string args_to_str(uint64_t args[], size_t size) {
  std::string res = "";
  size_t i = 0;

  if (size > 0) {
    for (; i < size-1; i++) {
      res += std::to_string(args[i]) + ",";
    }
    res += std::to_string(args[i]);
  }
  return res;
}

Value ExecutionVisitor::visit_number_range_atom(NumberRangeAtom *atom) {
  return Value(atom->type_, atom->list);
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_ifthenelse(IfThenElseNode* node) {
  Value cond = walk_expression_base(node->condition_);

  if (cond.value.bval) {
    walk_statement(node->then_);
  } else if (node->else_) {
    walk_statement(node->else_);
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_seqblock(UnaryNode* seqblock) {
  bool forked = false;
  if (visitor.context_.updateset.pseudostate % 2 == 0) {
    CASM_UPDATESET_FORK_SEQ(&visitor.context_.updateset);
    forked = true;
  }
  visitor.visit_seqblock(seqblock);
  walk_statements(reinterpret_cast<AstListNode*>(seqblock->child_));

  if (forked) {
    visitor.context_.merge_seq(visitor.driver_);
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_parblock(UnaryNode* parblock) {
  bool forked = false;
  if (visitor.context_.updateset.pseudostate % 2 == 1) {
    CASM_UPDATESET_FORK_PAR(&visitor.context_.updateset);
    forked = true;
  }
  visitor.visit_seqblock(parblock);
  walk_statements(reinterpret_cast<AstListNode*>(parblock->child_));

  if (forked) {
    visitor.context_.merge_par();
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_pop(PopNode* node) {
  // TODO this should use the same code as the global visitor
  Value from = walk_function_atom(node->from);
  visitor.visit_pop(node, from);
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_case(CaseNode *node) {
  Value expr_v = walk_expression_base(node->expr);
  std::pair<AtomNode*, AstNode*> *default_pair = nullptr;
  for (auto& pair : node->case_list) {
    // pair.first == nullptr for default:
    if (pair.first) {
      if (walk_atom(pair.first) == expr_v) {
        walk_statement(pair.second);
        return;
      }
    } else {
      default_pair = &pair;
    }
  }
  if (default_pair) {
    walk_statement(default_pair->second);
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_forall(ForallNode *node) {
  bool forked = false;
  Value in_list = walk_expression_base(node->in_expr);

  if (visitor.context_.updateset.pseudostate % 2 == 1) {
    CASM_UPDATESET_FORK_PAR(&visitor.context_.updateset);
    forked = true;
  }

  switch (node->in_expr->type_.t) {
    case TypeType::LIST: {
      List *l =  in_list.value.list;

      for (auto iter = l->begin(); iter != l->end(); iter++) {
        visitor.rule_bindings.back()->push_back(*iter);
        walk_statement(node->statement);
        visitor.rule_bindings.back()->pop_back();
      }
      break;
    }
    case TypeType::INT: {
      INT_T end =  in_list.value.ival;

      if (end > 0) {
        for (INT_T i = 0; i < end; i++) {
          visitor.rule_bindings.back()->push_back(Value(i));
          walk_statement(node->statement);
          visitor.rule_bindings.back()->pop_back();
        }
      } else {
        for (INT_T i = 0; end < i; i--) {
          visitor.rule_bindings.back()->push_back(Value(i));
          walk_statement(node->statement);
          visitor.rule_bindings.back()->pop_back();
        }
      }
      break;
    }
    case TypeType::ENUM: {
      FunctionAtom *func = reinterpret_cast<FunctionAtom*>(node->in_expr);
      if (func->name == func->enum_->name) {
        for (auto pair : func->enum_->mapping) {
          // why is an element with the name of the enum in the map??
          if (func->name == pair.first) {
            continue;
          }
          Value v = Value(pair.second);
          v.type = TypeType::ENUM;
          visitor.rule_bindings.back()->push_back(std::move(v));
          walk_statement(node->statement);
          visitor.rule_bindings.back()->pop_back();
        }
      } else {
        assert(0);
      }
      break;
    }
    default: assert(0);
  }

  if (forked) {
    visitor.context_.merge_par();
  }
}

void ExecutionWalker::run() {
  std::vector<uint64_t*> initializer_args;

  for (auto pair: visitor.context_.symbol_table.table_) {
    auto function_map = std::unordered_map<ArgumentsKey, Value>();

    if (pair.second->type != Symbol::SymbolType::FUNCTION) {
      continue;
    }

    Function *func = reinterpret_cast<Function*>(pair.second);
    if (func->intitializers_ != nullptr) {
      for (std::pair<ExpressionBase*, ExpressionBase*> init : *func->intitializers_) {
        size_t num_args = 0; 
        uint64_t *args = new uint64_t[10];
        if (init.first != nullptr) {
          std::vector<Value> arguments;
          Value argument_v = walk_expression_base(init.first);
          if (func->arguments_.size() > 1) {
            List *list = argument_v.value.list;
            for (auto iter = list->begin(); iter != list->end(); iter++) {
              arguments.push_back(*iter);
            }
          } else {
            arguments.push_back(argument_v);
          }
          pack_values_in_array(arguments, &args[0]);
          num_args = arguments.size();
        } else {
          args[0] = 0;
        }

        if (function_map.count({&args[0], num_args}) != 0) {
          yy::location loc = init.first ? init.first->location+init.second->location : init.second->location;
          visitor.driver_.error(loc, "function `"+pair.first+"("+args_to_str(args, num_args)+")` already initialized");
          throw RuntimeException("function already initialized");
        }
        function_map.emplace(std::pair<ArgumentsKey, Value>({&args[0], num_args}, walk_expression_base(init.second)));
        initializer_args.push_back(args);
      }
    }
    visitor.context_.functions[func->id] = std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>(func, function_map);
  }
  for (List *l : visitor.context_.temp_lists) {
    l->bump_usage();
  }

  visitor.context_.temp_lists.clear();

  Function *program_sym = visitor.context_.symbol_table.get_function("program");
  uint64_t args[10] = {0};
  size_t steps = 0;
  while(true) {
    Value& program_val = visitor.context_.get_function_value(program_sym, args);
    DEBUG(program_val.to_str());
    if (program_val.type == TypeType::UNDEF) {
      break;
    }
    walk_rule(program_val.value.rule);
    visitor.context_.apply_updates();
    steps += 1;
  }

  if (steps > 1) {
    std::cout << steps <<" steps later..."<<std::endl;
  } else {
    std::cout << steps <<" step later..."<<std::endl;
  }


  for (auto ptr : initializer_args) {
    delete[] ptr;
  }
}
