#include <algorithm>
#include <cmath>
#include <assert.h>
#include <utility>
#include <sstream>

#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"

DEFINE_CASM_UPDATESET_FORK_PAR
DEFINE_CASM_UPDATESET_FORK_SEQ

Value casm_pow(std::vector<Value> &expr_results) {
  switch (expr_results[0].type) {
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

Value casm_hex(std::vector<Value> &expr_results) {
  // TODO LEAK!
  if (expr_results[0].is_undef()) {
    return Value(new std::string("undef"));
  }

  std::stringstream ss;
  if (expr_results[0].value.ival < 0) {
    ss << "-" << std::hex << (-1) * expr_results[0].value.ival;
  } else {
    ss << std::hex << expr_results[0].value.ival;
  }
  return Value(new std::string(ss.str()));
}

Value casm_nth(std::vector<Value> &expr_results) {
  if (expr_results[0].is_undef()) {
    return Value();
  }

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

Value casm_cons(ExecutionContext& ctxt, const Value& val, const Value& list) {
  // TODO LEAK
  if (list.is_undef()) {
    return Value();
  }

  HeadList *consed_list = new HeadList(list.value.list, val);
  ctxt.temp_lists.push_back(consed_list);
  return Value(list.type, consed_list);
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

Value casm_tail(ExecutionContext& ctxt, const Value& arg_list) {
  if (arg_list.is_undef()) {
    return Value();
  }

  List *list = arg_list.value.list;

  if (list->is_head()) {
    return Value(arg_list.type, reinterpret_cast<HeadList*>(list)->right);
  } else if (list->is_bottom()) {
    BottomList *btm = reinterpret_cast<BottomList*>(list);
    if (btm->values.size() > 0) {
      SkipList *skip = new SkipList(1, btm);
      ctxt.temp_lists.push_back(skip);
      return Value(arg_list.type, skip);
    } else {
      // tail for empty list returns empty list
      return arg_list;
    }
  } else {
    SkipList *old_skip = reinterpret_cast<SkipList*>(list);
    SkipList *skip = new SkipList(old_skip->skip+1, old_skip->bottom);
    ctxt.temp_lists.push_back(skip);
    return Value(arg_list.type, skip);
  }
}

Value casm_peek(const Value arg) {
  if (arg.is_undef()) {
    return Value();
  }

  List *list = arg.value.list;

  if (list->begin() != list->end()) {
    return Value(*(list->begin()));
  } else {
    return Value();
  }
}


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
        if (call->rule->arguments[i]->t == TypeType::LIST) {
          // TODO
          assert(0);
        } else if (call->rule->arguments[i]->t == TypeType::LIST) {
          // TODO
          assert(0);
        } else if (*call->rule->arguments[i] != argument_results[i].type) {
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
  Value to_res = casm_cons(context_, expr, atom);

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
  Value to_res = casm_peek(val);

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

  Value from_res = casm_tail(context_, val);
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
  } else if (atom->name == "hex") {
    return casm_hex(expr_results);
  } else if (atom->name == "nth") {
    return casm_nth(expr_results);
  } else if (atom->name == "cons") {
    return casm_cons(context_, expr_results[0], expr_results[1]);
  } else if (atom->name == "len") {
    return casm_len(expr_results);
  } else if (atom->name == "tail") {
    return casm_tail(context_, expr_results[0]);
  } else if (atom->name == "peek") {
    return casm_peek(expr_results[0]);
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
  for (List *l : visitor.context_.temp_lists) {
    l->bump_usage();
  }

  visitor.context_.temp_lists.clear();

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
