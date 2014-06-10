#include <algorithm>
#include <sstream>
#include <cmath>
#include <assert.h>
#include <utility>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>


#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"
#include "libinterpreter/builtins.h"
#include "libinterpreter/operators.h"
#include "libinterpreter/symbolic.h"

IGNORE_VARIADIC_WARNINGS

DEFINE_CASM_UPDATESET_FORK_PAR
DEFINE_CASM_UPDATESET_FORK_SEQ

REENABLE_VARIADIC_WARNINGS


uint16_t pack_values_in_array(const std::vector<Value> &value_list, uint64_t array[]) {
  uint16_t sym_args = 0;
  for (size_t i=0; i < value_list.size(); i++) {
    array[i] = value_list[i].to_uint64_t();
    if (value_list[i].is_symbolic()) {
      sym_args = sym_args | (1 << i);
    }
  }
  return sym_args;
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
  up->symbolic = val.is_symbolic();
  up->func = sym_id;
  // TODO: Do we need line here?
  //up->line = (uint64_t) loc.lines;
  up->sym_args = pack_values_in_array(arguments, up->args);

  up->num_args = arguments.size();

  auto& function_map = context_.functions[sym_id];
  if (function_map.second.count(ArgumentsKey(up->args, up->num_args, false, up->sym_args)) == 0) {
    function_map.second.emplace(ArgumentsKey(up->args, up->num_args, true, up->sym_args), Value());
  }
  Value& ref = function_map.second[ArgumentsKey(up->args, up->num_args, false, up->sym_args)];
  casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                    (void*) &ref,
                                                    (void*) up);
  if (v != nullptr) {
    // Check if values match
    for (int i=0; i < up->num_args; i++) {
      if (!eq_uint64_value(function_map.first->arguments_[i], up->args[i], v->args[i])) {
        return up;
      }
    }
    throw RuntimeException("Conflict in updateset");
  }
  return up;
}

void ExecutionVisitor::visit_update_dumps(UpdateNode *update, Value& expr_v) {
  const std::string& filter = driver_.function_trace_map[update->func->symbol->id];
  if (context_.filter_enabled(filter)) {
    std::cout << filter << ": " << update->func->symbol->name ;

    auto iter = value_list.begin();
    if (iter != value_list.end()) {
      std::cout <<"("<< (*iter).to_str();
      iter++;
    }

    while (iter != value_list.end()) {
      std::cout << ", " << (*iter).to_str();
      iter++;
    }
    if (value_list.size() > 0) {
      std::cout << ")";
    }

    std::cout << " = "<< expr_v.to_str() << std::endl;
  }

  visit_update(update, expr_v);
}

void ExecutionVisitor::visit_update(UpdateNode *update, Value& expr_v) {
  try {
    casm_update *up = add_update(expr_v, update->func->symbol->id, value_list);
    up->line = (uint64_t) &update->location;
    value_list.clear();
    DEBUG("UPADTE "<<update->func->name<<" num args "<<up->num_args << " arg[0] "<<up->args[0]<< " val "<<expr_v.to_str());

    if (context_.symbolic && update->func->symbol->is_symbolic) {
      symbolic::dump_update(context_.trace, update->func->symbol, up->args, up->sym_args, expr_v);
    }
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
  std::stringstream ss;
  if (node->filter.size() > 0 ) {
    if (context_.filter_enabled(node->filter)) {
      ss << node->filter << ": ";
    } else {
      return;
    }
  }

  for (const Value& v: arguments) {
    ss << v.to_str();
  }
  ss << std::endl;

  if (context_.symbolic) {
    context_.trace.push_back(ss.str());
  } else {
    std::cout << ss.str();
  }
}

void ExecutionVisitor::visit_diedie(DiedieNode *node, const Value& msg) {
  if (node->msg) {
    driver_.error(node->location, *msg.value.string);
  } else {
    driver_.error(node->location, "`diedie` executed");
  }
    throw RuntimeException("diedie");
}

void ExecutionVisitor::visit_impossible(AstNode *node) {
  if (context_.symbolic) {
    driver_.info(node->location, "`impossible` executed, aborting trace");
    throw ImpossibleException();
  } else {
    driver_.error(node->location, "`impossible` executed");
    throw RuntimeException("impossible");
  }
}

void ExecutionVisitor::visit_let(LetNode*, Value& v) {
  rule_bindings.back()->push_back(v);
}

void ExecutionVisitor::visit_let_post(LetNode*) {
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
      uint16_t sym_args = pack_values_in_array(expr_results, args);

      Value v = Value(context_.get_function_value(atom->symbol, args, sym_args));
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
  // TODO Int2Enum is a special builtin, it needs the complete type information
  // for the enum, values only store TypeType and passing the type to all
  // builtins seems ugly.
  // Maybe store Type* in Value?
  if (atom->id == BuiltinAtom::Id::INT2ENUM) {
    Enum *enum_ = context_.symbol_table.get_enum(atom->type_.enum_name);
    for (auto pair : enum_->mapping) {
      // TODO check why the enum mapping contains an extra entry with the name
      // of the enum
      if (pair.first != enum_->name && pair.second->id == expr_results[0].value.ival) {
        return std::move(Value(pair.second));
      }
    }
    return std::move(Value());
  }

  return builtins::dispatch(atom->id, context_, expr_results);
}

void ExecutionVisitor::visit_derived_function_atom_pre(FunctionAtom*, std::vector<Value>& arguments) {
  rule_bindings.push_back(&arguments);
}

Value ExecutionVisitor::visit_derived_function_atom(FunctionAtom*, Value& expr) {
  rule_bindings.pop_back();
  return expr;
}

Value ExecutionVisitor::visit_list_atom(ListAtom *atom, std::vector<Value> &vals, bool symbolic) {
  BottomList *list = new BottomList(vals);
  // this could be faster if the list of expressions would be evaluated back to
  // front as well
  std::reverse(list->values.begin(), list->values.end());
  //context_.temp_lists.push_back(list);

  DEBUG("LIST ATOM "<<symbolic);
  if (symbolic) {
    uint32_t sym_id = symbolic::dump_listconst(context_.trace_creates, list);
    if (sym_id > 0) {
      return Value(TypeType::SYMBOL, sym_id);
    }
  }
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

ExpressionOperation invert(ExpressionOperation op) {
  switch (op) {
    case ExpressionOperation::EQ: return ExpressionOperation::NEQ;
    case ExpressionOperation::NEQ: return ExpressionOperation::EQ;
    case ExpressionOperation::LESSEREQ: return ExpressionOperation::GREATER;
    case ExpressionOperation::LESSER: return ExpressionOperation::GREATEREQ;
    case ExpressionOperation::GREATER: return ExpressionOperation::LESSEREQ;
    case ExpressionOperation::GREATEREQ: return ExpressionOperation::LESSER;
    default: throw RuntimeException("Invert not implemented for operation");
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_ifthenelse(IfThenElseNode* node) {
  Value cond = walk_expression_base(node->condition_);

  if (cond.is_symbolic()) {
    symbolic_condition *sym_cond;
    if (cond.type == TypeType::SYMBOL_COND) {
      sym_cond = cond.value.cond;
    } else {
      sym_cond = new symbolic_condition(new Value(cond), new Value((INT_T)1), ExpressionOperation::EQ);
    }

    switch (symbolic::check_condition(visitor.context_.path_conditions, sym_cond)) {
      case symbolic::check_status_t::NOT_FOUND: break;
      case symbolic::check_status_t::TRUE:
        symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, cond, true);
        walk_statement(node->then_);
        return;
      case symbolic::check_status_t::FALSE:;
        symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, cond, false);

        if (node->else_) {
          walk_statement(node->else_);
        }
        return;
    }

    pid_t pid = fork();
    switch (pid) {
      case -1:
        throw RuntimeException("Could not fork");

      case 0:
        visitor.context_.path_name += "I";
        symbolic::dump_if(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, Value(sym_cond));
        visitor.context_.path_conditions.push_back(sym_cond);
        walk_statement(node->then_);
        break;

      default: {
        // at the moment this limits parallelism, but ensures a deterministic
        // trace output on stdout
        int status;
        if (waitpid(pid, &status, 0) == -1) {
          throw RuntimeException("error waiting for child process");
        }
        if (WEXITSTATUS(status) != 0) {
          throw RuntimeException("error in child process");
        }

        if (cond.type == TypeType::SYMBOL_COND) {
          sym_cond->op = invert(sym_cond->op);
        } else {
          // needed to generate correct output for boolean functions as conditions
          delete sym_cond;
          sym_cond = new symbolic_condition(new Value(cond),
              new Value((INT_T)0), ExpressionOperation::EQ);
        }
        visitor.context_.path_name += "E";
        symbolic::dump_if(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, Value(sym_cond));
        visitor.context_.path_conditions.push_back(sym_cond);
        if (node->else_) {
          walk_statement(node->else_);
        }
      }
    }
  } else if (cond.is_undef()) {
    visitor.driver_.error(node->condition_->location,
        "condition must be true or false but was undef");
    throw RuntimeException("Condition is undef");
  } else if (cond.value.bval) {
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
  Value cond = walk_expression_base(node->expr);

  if (cond.is_symbolic()) {
    for (uint32_t i=0; i < node->case_list.size(); i++) {
      auto pair = node->case_list[i];
      // pair.first == nullptr for default:
      symbolic_condition *sym_cond;
      if (pair.first) {
        Value c = walk_atom(pair.first);
        sym_cond = new symbolic_condition(new Value(cond), new Value(c),
            ExpressionOperation::EQ);

        switch (symbolic::check_condition(visitor.context_.path_conditions, sym_cond)) {
          case symbolic::check_status_t::NOT_FOUND: break;
          case symbolic::check_status_t::TRUE:
            symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
                pair.first->location.begin.line, cond, true);
            walk_statement(pair.second);
            return;
          default: break;
        }
      }

      pid_t pid = fork();
      switch (pid) {
        case -1:
          throw RuntimeException("Could not fork");

        case 0: {
          if (pair.first) {
            visitor.context_.path_name += std::to_string(i);
            visitor.context_.path_conditions.push_back(sym_cond);
            symbolic::dump_if(visitor.context_.trace, visitor.driver_.get_filename(),
              pair.first->location.begin.line, Value(sym_cond));
          } else {
            visitor.context_.path_name += "D";
          }
          walk_statement(pair.second);
          return;
        }
        default: {
          // at the moment this limits parallelism, but ensures a deterministic
          // trace output on stdout
          int status;
          if (waitpid(pid, &status, 0) == -1) {
            throw RuntimeException("error waiting for child process");
          }
          if (WEXITSTATUS(status) != 0) {
            throw RuntimeException("error in child process");
          }
        }
      }
    }
    exit(0);
  } else {
    std::pair<AtomNode*, AstNode*> *default_pair = nullptr;
    for (auto& pair : node->case_list) {
      // pair.first == nullptr for default:
      if (pair.first) {
        if (walk_atom(pair.first) == cond) {
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

DEFINE_CASM_UPDATESET_EMPTY

template <>
void AstWalker<ExecutionVisitor, Value>::walk_iterate(UnaryNode *node) {
  bool forked = false;
  bool running = true;

  if (visitor.context_.updateset.pseudostate % 2 == 0) {
    CASM_UPDATESET_FORK_SEQ(&visitor.context_.updateset);
    forked = true;
  }

  while (running) {
    CASM_UPDATESET_FORK_PAR(&visitor.context_.updateset);

    walk_statement(node->child_);
    if (CASM_UPDATESET_EMPTY(&visitor.context_.updateset)) {
      running = false;
    }
    visitor.context_.merge_par();
  }

  if (forked) {
    visitor.context_.merge_seq(visitor.driver_);
  }
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_update(UpdateNode *node) {
  // this is used to dump %CREATE in trace if necessary
  Value expr_t;
  if (visitor.context_.symbolic && node->func->symbol->is_symbolic) {
    expr_t = walk_expression_base(node->expr_, true);
    walk_expression_base(node->func);
  } else {
    expr_t = walk_expression_base(node->expr_, false);
  }

  visitor.value_list.clear();
  if (node->func->arguments) {
    for (ExpressionBase* e : *node->func->arguments) {
      visitor.value_list.push_back(walk_expression_base(e));
    }
  }

  visitor.visit_update(node, expr_t);
}

template <>
void AstWalker<ExecutionVisitor, Value>::walk_update_dumps(UpdateNode *node) {
  Value expr_t = walk_expression_base(node->expr_);
  
  visitor.value_list.clear();
  if (node->func->arguments) {
    for (ExpressionBase* e : *node->func->arguments) {
      visitor.value_list.push_back(walk_expression_base(e));
    }
  }

  visitor.visit_update_dumps(node, expr_t);
}

ExecutionWalker::ExecutionWalker(ExecutionVisitor& v) 
     : AstWalker<ExecutionVisitor, Value>(v), initialized() {
}

bool ExecutionWalker::init_function(const std::string& name, std::set<std::string>& visited) {
  if (visitor.driver_.init_dependencies.count(name) != 0) {
    visited.insert(name);
    const std::set<std::string>& deps = visitor.driver_.init_dependencies[name];
    for (const std::string& dep : deps) {
      if (visited.count(dep) > 0) {
        return false;
      } else {
        if (!init_function(dep, visited)) {
          return false;
        }
      }
    }
  }

  std::vector<uint64_t*> initializer_args;

  Function *func = visitor.context_.symbol_table.get_function(name);
  if (!func) {
    return true;
  }

  visitor.context_.functions[func->id] = std::move(std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>(func,std::unordered_map<ArgumentsKey, Value>(0, {func->arguments_}, {func->arguments_})));
  auto& function_map = visitor.context_.functions[func->id].second;

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

      if (function_map.count(ArgumentsKey(&args[0], num_args, false, 0)) != 0) {
        yy::location loc = init.first ? init.first->location+init.second->location : init.second->location;
        visitor.driver_.error(loc, "function `"+func->name+"("+args_to_str(args, num_args)+")` already initialized");
        throw RuntimeException("function already initialized");
      }
      DEBUG("BEGIN INSERT "<<name);

      if (visitor.context_.symbolic && func->is_symbolic) {
        symbolic::dump_create(visitor.context_.trace_creates, func,
            &args[0], 0, walk_expression_base(init.second));
      }

      function_map.emplace(std::pair<ArgumentsKey, Value>(std::move(ArgumentsKey(&args[0], num_args, true, 0)), walk_expression_base(init.second)));
      DEBUG("END INSERT "<<name);
      initializer_args.push_back(args);
    }
  }

  initialized.insert(name);
  return true;
}

void ExecutionWalker::run() {

  for (auto pair : visitor.driver_.init_dependencies) {
    std::set<std::string> visited;
    if (initialized.count(pair.first) > 0) {
      continue;;
    }
    if (!init_function(pair.first, visited)) {
      Function *func = visitor.context_.symbol_table.get_function(pair.first);
      std::string cycle = pair.first;
      for (const std::string& dep : visited) {
        cycle = cycle + " => " + dep;
      }
      visitor.driver_.error(func->intitializers_->at(0).second->location, "initializer dependency cycle detected: "+cycle);
      throw RuntimeException("Initializer cycle");
    }
  }


  for (auto pair: visitor.context_.symbol_table.table_) {
    if (pair.second->type != Symbol::SymbolType::FUNCTION || initialized.count(pair.first) > 0) {
      continue;
    }

    std::set<std::string> visited;
    init_function(pair.first, visited);
  }

  for (List *l : visitor.context_.temp_lists) {
    l->bump_usage();
  }

  visitor.context_.temp_lists.clear();

  Function *program_sym = visitor.context_.symbol_table.get_function("program");
  uint64_t args[10] = {0};
  while(true) {
    Value& program_val = visitor.context_.get_function_value(program_sym, args, 0);
    DEBUG(program_val.to_str());
    if (program_val.type == TypeType::UNDEF) {
      break;
    }
    walk_rule(program_val.value.rule);
    visitor.context_.apply_updates();
    // reuse symbolic counter as step counter, saves one counter in the main
    // loop
    symbolic::advance_timestamp();
  }

  if (visitor.context_.symbolic) {
    FILE *out;
    if (visitor.context_.fileout) {
      const std::string& filename = visitor.driver_.get_filename().substr(
          0, visitor.driver_.get_filename().rfind("."));

      out = fopen((filename+"_"+visitor.context_.path_name+".trace").c_str(), "wt");
    } else {
      out = stdout;
    }
    fprintf(out, "forklog:%s\n", visitor.context_.path_name.c_str());
    uint32_t fof_id = 0;
    for (const std::string& s : visitor.context_.trace_creates) {
      if (s.find("id%u") != std::string::npos) {
        fprintf(out, s.c_str(), fof_id);
        fof_id += 1;
      } else {
        fprintf(out, "%s", s.c_str());
      }
    }
    symbolic::dump_final(visitor.context_.trace, visitor.context_.functions);
    for (const std::string& s : visitor.context_.trace) {
     if (s.find("id%u") != std::string::npos) {
        fprintf(out, s.c_str(), fof_id);
        fof_id += 1;
      } else {
        fprintf(out, "%s", s.c_str());
      }
    }
    fprintf(out, "\n");
  } else {
    std::cout << (symbolic::get_timestamp()-2);
    if ((symbolic::get_timestamp()-2) > 1) {
      std::cout << " steps later..." << std::endl;
    } else {
      std::cout << " step later..." << std::endl;
    }
  }
}
