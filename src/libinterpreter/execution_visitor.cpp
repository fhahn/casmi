#include <cmath>
#include <assert.h>
#include <utility>

#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"

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

void ExecutionVisitor::visit_update(UpdateNode *update, Value &func_val, Value& expr_v) {
  UNUSED(func_val);

  casm_update* up = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

  up->value = (void*) expr_v.to_uint64_t();
  up->defined = (expr_v.type == TypeType::UNDEF) ? 0 : 1;
  up->func = update->func->symbol->id;
  up->line = (uint64_t) update;
  pack_values_in_array(value_list, up->args);

  up->num_args = value_list.size();
  value_list.clear();

  auto& function_map = context_.functions[update->func->symbol->id];
  if (function_map.second.count({up->args, up->num_args}) == 0) {
    function_map.second.emplace(ArgumentsKey{up->args, up->num_args}, Value());
  }
  Value& ref = function_map.second[{up->args, up->num_args}];
  casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                    (void*) &ref,
                                                    (void*) up);
  DEBUG("UPADTE "<<update->func->name<<" num args "<<up->num_args << " arg[0] "<<up->args[0]<< " val "<<expr_v.to_str());
  if (v != nullptr) {
    // Check if values match
    for (int i=0; i < up->num_args; i++) {
      if (up->args[i] != v->args[i]) {
        return;
      }
    }

    driver_.error(update->func->location,
                  "update conflict in parallel block for function `"+update->func->name+"`");
    throw RuntimeException("Conflict in updateset");
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
        if (*call->rule->arguments[i] != argument_results[i].type) {
          driver_.error(call->arguments->at(i)->location,
                        "argument "+std::to_string(i+1)+" of indirectly called rule `"+
                        call->rule->name+"` must be `"+
                        call->rule->arguments[i]->to_str()+"` but was `"+
                        argument_results[i].type.to_str()+"`");
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

Value ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  DEBUG("left "<<left_val.to_str()<<" right "<<right_val.to_str());
  switch (expr->op) {
    case Expression::Operation::ADD: {
      left_val.add(right_val);
      DEBUG("ADD "<<left_val.to_str());
      return left_val;
    }
    case Expression::Operation::SUB: {
      left_val.sub(right_val);
      return left_val;
    }
    case Expression::Operation::MUL: {
      left_val.mul(right_val);
      return left_val;
    }
    case Expression::Operation::DIV: {
      left_val.div(right_val);
      return left_val;
    }
    case Expression::Operation::MOD: {
      left_val.mod(right_val);
      return left_val;
    }
    case Expression::Operation::RAT_DIV: {
      left_val.mod(right_val);
      return left_val;
    }

    case Expression::Operation::EQ: {
      Value tmp(value_eq(left_val, right_val));
      return tmp;
    }
    case Expression::Operation::NEQ: {
      Value tmp(!value_eq(left_val, right_val));
      return tmp;
    }

    case Expression::Operation::LESSER:
      left_val.lesser(right_val);
      return left_val;
    case Expression::Operation::GREATER:
      left_val.greater(right_val);
      return left_val;
    case Expression::Operation::LESSEREQ:
      left_val.lessereq(right_val);
      return left_val;
    case Expression::Operation::GREATEREQ:
      left_val.greatereq(right_val);
      return left_val;



    default: assert(0);
  }
}

Value ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  UNUSED(expr);
  return val;
}

Value casm_pow(std::vector<Value> &expr_results) {
  switch (expr_results[0].type.t) {
    case TypeType::INT:
      return Value((INT_T)std::pow(expr_results[0].value.ival,
                                             expr_results[1].value.ival));
    case TypeType::FLOAT:
      DEBUG("POW ARGS "<<expr_results[0].value.fval<<" foobar "<<expr_results[1].value.fval);
      return Value((FLOAT_T)std::pow(expr_results[0].value.fval,
                                             expr_results[1].value.fval));

    default: assert(0);

  }
}

Value casm_nth(std::vector<Value> &expr_results) {
  List *list = expr_results[0].value.list;
  List::const_iterator iter = list->begin();
  size_t i = 1;

  while (iter != list->end() && i < expr_results[1].value.ival) {
    i++;
    iter++;
  }
  if (i == expr_results[1].value.ival && iter != list->end()) {
    return Value(*iter);
  } else {
    return Value();
  }
}

Value casm_cons(std::vector<Value> &expr_results) {
  // TODO LEAK
  if (expr_results[1].is_undef()) {
    return Value();
  }

  TempList *consed_list = new TempList();
  consed_list->changes.push_back(expr_results[0]);
  consed_list->right = expr_results[1].value.list;
  return Value(expr_results[1].type, consed_list);
}

Value casm_len(std::vector<Value> &expr_results) {
  // TODO len is really slow right now, it itertes over complete list
  if (expr_results[0].is_undef()) {
    return Value();
  }

  List *list = expr_results[0].value.list;
  List::const_iterator iter = list->begin();

  size_t count = 0;

  while (iter != list->end()) {
    count++;
    iter++;
  }

  return Value((INT_T) count);
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
      DEBUG("visit_atom "<<atom->symbol->name()<<" "<<v.to_str() <<" size "<<value_list.size());
      return v;
    }
    default:
      assert(0);
  }
}

Value ExecutionVisitor::visit_builtin_atom(BuiltinAtom *atom, std::vector<Value> &expr_results) {
  if (atom->name == "pow") {
    return casm_pow(expr_results);
  } else if (atom->name == "nth") {
    // TODO check if move happens here, why do we need a tmp var v here?
    Value v = casm_nth(expr_results);
    return v;
  } else if (atom->name == "cons") {
    return casm_cons(expr_results);
  } else if (atom->name == "len") {
    return casm_len(expr_results);
  } else {
    assert(0);
  }
}

Value ExecutionVisitor::visit_derived_function_atom(FunctionAtom *atom,
                                                      std::vector<Value> &expr_results,
                                                      Value& expr) {
  return expr;
}

Value ExecutionVisitor::visit_list_atom(ListAtom *atom, std::vector<Value> &vals) {
  atom->tmp_list.changes = std::move(vals);
  return Value(atom->type_, &atom->tmp_list);
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


void ExecutionWalker::run() {
  std::vector<uint64_t*> initializer_args;

  for (auto pair: visitor.context_.symbol_table.table_) {
    auto function_map = std::unordered_map<ArgumentsKey, Value>();

    if (pair.second->symbol_type == Function::SType::FUNCTION && pair.second->intitializers_ != nullptr) {
      for (std::pair<ExpressionBase*, ExpressionBase*> init : *pair.second->intitializers_) {
        size_t num_args = 0; 
        uint64_t *args = new uint64_t[10];
        if (init.first != nullptr) {
          std::vector<Value> ident;
          ident.push_back(walk_expression_base(init.first));
          pack_values_in_array(ident, &args[0]);
          num_args = ident.size();
          DEBUG("INTI FOO "<<pair.second->id << " arg: "<<args[0]);
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
    visitor.context_.functions[pair.second->id] = std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>(pair.second, function_map);
  }

  Function *program_sym = visitor.context_.symbol_table.get("program");
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

  std::cout << steps <<" steps later..."<<std::endl;


  for (auto ptr : initializer_args) {
    delete[] ptr;
  }
}
