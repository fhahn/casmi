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


uint16_t pack_values_in_array(const value_t value_list[], uint64_t array[], uint32_t size) {
  uint16_t sym_args = 0;
  for (uint32_t i=0; i < size; i++) {
    const value_t& v = value_list[i];
    array[i] = v.to_uint64_t();
    if (v.is_symbolic()) {
      sym_args = sym_args | (1 << i);
    }
  }
  return sym_args;
}


ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, Driver& driver)
    : driver_(driver), context_(ctxt) {
  rule_bindings.push_back(&main_bindings);
}

void ExecutionVisitor::visit_assert(UnaryNode* assert, const value_t& val) {
  if (val.value.boolean != true) {
    driver_.error(assert->location,
                  "Assertion failed");
    throw RuntimeException("Assertion failed");
  }
}

void ExecutionVisitor::visit_assure(UnaryNode* assure, const value_t& val) {
  if (context_.symbolic && val.is_symbolic() && val.value.sym->condition) {
    context_.path_conditions.push_back(val.value.sym->condition);
  } else {
    visit_assert(assure, val);
  }
}

casm_update *ExecutionVisitor::add_update(const value_t& val, size_t sym_id) {
  casm_update* up = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

  up->value = (void*) val.to_uint64_t();
  up->defined = (val.is_undef()) ? 0 : 1;
  up->symbolic = val.is_symbolic();
  up->func = sym_id;
  // TODO: Do we need line here?
  //up->line = (uint64_t) loc.lines;
  // TODO use arg!
  up->sym_args = pack_values_in_array(arguments, up->args, num_arguments);

  up->num_args = num_arguments;

  auto& function_map = context_.function_states[sym_id];
  const ArgumentsKey key(up->args, up->num_args, false, up->sym_args);
  if (function_map.count(key) == 0) {
    function_map.emplace(ArgumentsKey(up->args, up->num_args, true, up->sym_args), value_t());
  }
  const value_t& ref = function_map[key];

  casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                    (void*) &ref,
                                                    (void*) up);

  if (v != nullptr) {
    // Check if values match
    const Function* function_symbol = context_.function_symbols[sym_id];
    for (int i=0; i < up->num_args; i++) {
      if (!eq_uint64_value(function_symbol->arguments_[i], up->args[i], v->args[i])) {
        return up;
      }
    }
    throw RuntimeException("Conflict in updateset");
  }
  return up;
}

void ExecutionVisitor::visit_update_dumps(UpdateNode *update, const value_t& expr_v) {
  const std::string& filter = driver_.function_trace_map[update->func->symbol->id];
  if (context_.filter_enabled(filter)) {
    std::cout << filter << ": " << update->func->symbol->name ;

    if (num_arguments > 0) {
      std::cout <<"("<< arguments[0].to_str();
    }

    for (uint16_t i=1; i<  num_arguments; i++) {
      std::cout << ", " << arguments[i].to_str();
    }
    if (num_arguments > 0) {
      std::cout << ")";
    }

    std::cout << " = "<< expr_v.to_str() << std::endl;
  }

  visit_update(update, expr_v);
}

void ExecutionVisitor::visit_update(UpdateNode *update, const value_t& expr_v) {
  try {
    casm_update *up = add_update(expr_v, update->func->symbol->id);
    up->line = (uint64_t) &update->location;
  } catch (const RuntimeException& ex) {
    // TODO this is probably not the cleanest solutions
    driver_.error(update->location,
                  "update conflict in parallel block for function `"+update->func->name+"`");
    throw ex;
  }
}

void ExecutionVisitor::visit_update_subrange(UpdateNode *update, const value_t& expr_v) {
  INT_T v = expr_v.value.integer;
  Type *t = update->func->symbol->return_type_;
  if ((t->subrange_start < t->subrange_end) &&
      (v < t->subrange_start || v > t->subrange_end)) {
    driver_.error(update->location,
                  std::to_string(v)+" does violate the subrange "
                  +std::to_string(t->subrange_start)
                  +".." +std::to_string(t->subrange_end)
                  +" of `"+update->func->name+"`");
    throw RuntimeException("Subrange violated");
  }
  visit_update(update, expr_v);
}



void ExecutionVisitor::visit_call_pre(CallNode *call) { UNUSED(call); }

void ExecutionVisitor::visit_call_pre(CallNode *call, const value_t& expr) {
  if (expr.type != TypeType::UNDEF) {
    call->rule = expr.value.rule;
  } else {
    throw RuntimeException("Cannot call UNDEF");
  }
}

void ExecutionVisitor::visit_call(CallNode *call, std::vector<value_t> &argument_results) {
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

void ExecutionVisitor::visit_print(PrintNode *node, const std::vector<value_t> &arguments) {
  std::stringstream ss;
  if (node->filter.size() > 0 ) {
    if (context_.filter_enabled(node->filter)) {
      ss << node->filter << ": ";
    } else {
      return;
    }
  }

  for (const value_t& v: arguments) {
    ss << v.to_str();
  }
  ss << std::endl;

  if (context_.symbolic) {
    context_.trace.push_back(ss.str());
  } else {
    std::cout << ss.str();
  }
}

void ExecutionVisitor::visit_diedie(DiedieNode *node, const value_t& msg) {
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

void ExecutionVisitor::visit_let(LetNode*, const value_t& v) {
  rule_bindings.back()->push_back(v);
}

void ExecutionVisitor::visit_let_post(LetNode*) {
  rule_bindings.back()->pop_back();
}

void ExecutionVisitor::visit_push(PushNode *node, const value_t& expr, const value_t& atom) {
  // at the moment, functions with arguments are not supported
  num_arguments = 0;
  if (atom.is_symbolic()) {
    const value_t to_res(new symbol_t(symbolic::next_symbol_id()));
    if (atom.value.sym->list) {
      to_res.value.sym->list = builtins::cons(context_, expr,
          value_t(TypeType::LIST, atom.value.sym->list)).value.list;
    } else {
      to_res.value.sym->list = builtins::cons(context_, expr,
          value_t(TypeType::LIST, new BottomList())).value.list;
    }

    try {
      casm_update *up = add_update(to_res, node->to->symbol->id);
      up->line = (uint64_t) &node->location;
    } catch (const RuntimeException& ex) {
      // TODO this is probably not the cleanest solutions
      driver_.error(node->to->location,
                    "update conflict in parallel block for function `"+node->to->name+"`");
      throw ex;
    }

    value_t args[] = {atom, expr};
    symbolic::dump_builtin(context_.trace, "push", args, 2, to_res);
  } else {
    const value_t to_res = builtins::cons(context_, expr, atom);

    try {
      casm_update *up = add_update(to_res, node->to->symbol->id);
      up->line = (uint64_t) &node->location;
    } catch (const RuntimeException& ex) {
      // TODO this is probably not the cleanest solutions
      driver_.error(node->to->location,
                    "update conflict in parallel block for function `"+node->to->name+"`");
      throw ex;
    }
  }
}

void ExecutionVisitor::visit_pop(PopNode *node, const value_t& val) {
  // at the moment, functions with arguments are not supported
  num_arguments = 0;
  if (val.is_symbolic()) {
    const value_t to_res = (val.value.sym->list) ? builtins::peek(value_t(TypeType::LIST, val.value.sym->list)) :
                                           value_t(new symbol_t(symbolic::next_symbol_id()));

    casm_update *up = nullptr;
    if (node->to->symbol_type == FunctionAtom::SymbolType::FUNCTION) {
      try {
        up = add_update(to_res, node->to->symbol->id);
        up->line = (uint64_t) &node->location;
      } catch (const RuntimeException& ex) {
        // TODO this is probably not the cleanest solutions
        driver_.error(node->to->location,
                      "update conflict in parallel block for function `"+node->to->name+"`");
        throw ex;
      }
    } else {
      rule_bindings.back()->push_back(to_res);
    }

    const value_t from_res(new symbol_t(symbolic::next_symbol_id()));
    if (val.value.sym->list) {
      from_res.value.sym->list = builtins::tail(context_, 
          value_t(TypeType::LIST, val.value.sym->list)).value.list;
    }

    value_t args[] = {val, to_res};
    symbolic::dump_builtin(context_.trace, "pop", args, 2, from_res);

    try {
      up = add_update(from_res, node->from->symbol->id);
      up->line = (uint64_t) &node->location;
    } catch (const RuntimeException& ex) {
      // TODO this is probably not the cleanest solutions
      driver_.error(node->location,
                    "update conflict in parallel block for function `"+node->from->name+"`");
      throw ex;
    }

  } else {
    const value_t to_res = builtins::peek(val);

    if (node->to->symbol_type == FunctionAtom::SymbolType::FUNCTION) {
      try {
        casm_update *up = add_update(to_res, node->to->symbol->id);
        up->line = (uint64_t) &node->location;
      } catch (const RuntimeException& ex) {
        // TODO this is probably not the cleanest solutions
        driver_.error(node->to->location,
                      "update conflict in parallel block for function `"+node->to->name+"`");
        throw ex;
      }
    } else {
      rule_bindings.back()->push_back(to_res);
    } 

    const value_t from_res = builtins::tail(context_, val);
    try {
      casm_update *up = add_update(from_res, node->from->symbol->id);
      up->line = (uint64_t) &node->location;
    } catch (const RuntimeException& ex) {
      // TODO this is probably not the cleanest solutions
      driver_.error(node->location,
                    "update conflict in parallel block for function `"+node->from->name+"`");
      throw ex;
    }

  }
}

const value_t ExecutionVisitor::visit_expression(Expression *expr,
                                               const value_t &left_val,
                                               const value_t &right_val) {
  return operators::dispatch(expr->op, left_val, right_val);
}

value_t ExecutionVisitor::visit_expression_single(Expression *expr, const value_t &val) {
  UNUSED(expr);
  return operators::dispatch(expr->op, val, val);
}

const value_t ExecutionVisitor::visit_function_atom(FunctionAtom *atom,
                                                    const value_t arguments[],
                                                    uint16_t num_arguments) {
  switch (atom->symbol_type) {
    case FunctionAtom::SymbolType::PARAMETER:
      return value_t(rule_bindings.back()->at(atom->offset));

    case FunctionAtom::SymbolType::FUNCTION: {
      uint64_t args[5];
      uint16_t sym_args = pack_values_in_array(arguments, args, num_arguments);

      return context_.get_function_value(atom->symbol, args, sym_args);
    }
    case FunctionAtom::SymbolType::ENUM: {
      enum_value_t *val = atom->enum_->mapping[atom->name];
      value_t v = value_t(val);
      v.type = TypeType::ENUM;
      return v;
    }
    default: {
      FAILURE();
    }
  }
}

const value_t ExecutionVisitor::visit_function_atom_subrange(FunctionAtom *atom,
                                                             const value_t arguments[],
                                                             uint16_t num_arguments) {
  for (uint32_t i=0; i < atom->symbol->subrange_arguments.size(); i++) {
    uint32_t j = atom->symbol->subrange_arguments[i];
    value_t v = arguments[j];
    Type *t = atom->symbol->arguments_[j];
    if (v.value.integer < t->subrange_start ||
        v.value.integer > t->subrange_end) {
      driver_.error(atom->location,
                  std::to_string(v.value.integer)+" does violate the subrange "
                  +std::to_string(t->subrange_start)
                  +".." +std::to_string(t->subrange_end)
                  +" of "+std::to_string(i+1)+". function argument");
      throw RuntimeException("Subrange violated");
    }
  }
  return visit_function_atom(atom, arguments, num_arguments);
}


const value_t ExecutionVisitor::visit_builtin_atom(BuiltinAtom *atom,
                                                   const value_t arguments[],
                                                   uint16_t num_arguments) {
  // TODO Int2Enum is a special builtin, it needs the complete type information
  // for the enum, values only store TypeType and passing the type to all
  // builtins seems ugly.
  // Maybe store Type* in value_t?
  if (atom->id == BuiltinAtom::Id::INT2ENUM) {
    Enum *enum_ = context_.symbol_table.get_enum(atom->type_.enum_name);
    for (auto pair : enum_->mapping) {
      // TODO check why the enum mapping contains an extra entry with the name
      // of the enum
      if (pair.first != enum_->name && pair.second->id == arguments[0].value.integer) {
        return std::move(value_t(pair.second));
      }
    }
    return std::move(value_t());
  }

  return builtins::dispatch(atom->id, context_, arguments, num_arguments);
}

void ExecutionVisitor::visit_derived_function_atom_pre(FunctionAtom*,
                                                       const value_t arguments[],
                                                       uint16_t num_arguments) {
  // TODO change, cleanup!
  std::vector<value_t> *tmp = new std::vector<value_t>();
  for (uint32_t i=0; i < num_arguments; i++) {
    tmp->push_back(arguments[i]);
  }

  rule_bindings.push_back(tmp);
}

const value_t ExecutionVisitor::visit_derived_function_atom(FunctionAtom*, const value_t& expr) {
  rule_bindings.pop_back();
  return expr;
}

const value_t ExecutionVisitor::visit_list_atom(ListAtom *atom,
                                                const std::vector<value_t> &vals) {
  BottomList *list = new BottomList(vals);
  //context_.temp_lists.push_back(list);

  if (context_.symbolic) {
    uint32_t sym_id = symbolic::dump_listconst(context_.trace_creates, list);
    if (sym_id > 0) {
      // TODO cleanup symbols
      symbol_t *sym = new symbol_t(sym_id);
      sym->type_dumped = true;
      sym->list = list;
      const value_t v(sym);
      return v;
    }
  }
  return value_t(atom->type_, list);
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

const value_t ExecutionVisitor::visit_number_range_atom(NumberRangeAtom *atom) {
  return value_t(atom->type_, atom->list);
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
value_t AstWalker<ExecutionVisitor, value_t>::walk_list_atom(ListAtom *atom) {
  std::vector<value_t> expr_results;
  if (atom->expr_list) {
    for (auto iter=atom->expr_list->rbegin(); iter != atom->expr_list->rend(); iter++) {
      expr_results.push_back(walk_expression_base(*iter));
    }
  }
  return visitor.visit_list_atom(atom, expr_results);
}


template <>
void AstWalker<ExecutionVisitor, value_t>::walk_ifthenelse(IfThenElseNode* node) {
  const value_t cond = walk_expression_base(node->condition_);

  if (cond.is_symbolic()) {
    symbolic_condition_t *sym_cond;
    if (cond.value.sym->condition) {
      sym_cond = cond.value.sym->condition;
    } else {
      sym_cond = new symbolic_condition_t(new value_t(cond), new value_t((INT_T)1), ExpressionOperation::EQ);
    }

    switch (symbolic::check_condition(visitor.context_.path_conditions, sym_cond)) {
      case symbolic::check_status_t::NOT_FOUND: break;
      case symbolic::check_status_t::TRUE:
        symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, sym_cond, true);
        walk_statement(node->then_);
        return;
      case symbolic::check_status_t::FALSE:;
        symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, sym_cond, false);

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
            node->condition_->location.begin.line, sym_cond);
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

        if (cond.value.sym->condition) {
          sym_cond->op = invert(sym_cond->op);
        } else {
          // needed to generate correct output for boolean functions as conditions
          delete sym_cond;
          sym_cond = new symbolic_condition_t(new value_t(cond),
              new value_t((INT_T)0), ExpressionOperation::EQ);
        }
        visitor.context_.path_name += "E";
        symbolic::dump_if(visitor.context_.trace, visitor.driver_.get_filename(),
            node->condition_->location.begin.line, sym_cond);
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
  } else if (cond.value.boolean) {
    walk_statement(node->then_);
  } else if (node->else_) {
    walk_statement(node->else_);
  }
}

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_seqblock(UnaryNode* seqblock) {
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
void AstWalker<ExecutionVisitor, value_t>::walk_parblock(UnaryNode* parblock) {
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
void AstWalker<ExecutionVisitor, value_t>::walk_pop(PopNode* node) {
  const value_t from = walk_function_atom(node->from);
  if (visitor.context_.symbolic &&
      node->to->symbol_type == FunctionAtom::SymbolType::FUNCTION &&
      node->to->symbol->is_symbolic) {
    walk_function_atom(node->to);
  }
  visitor.visit_pop(node, from);
}

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_push(PushNode *node) {
  const value_t expr = walk_expression_base(node->expr);
  const value_t atom = walk_function_atom(node->to);
  visitor.visit_push(node, expr, atom);
}

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_case(CaseNode *node) {
  const value_t cond = walk_expression_base(node->expr);

  if (cond.is_symbolic()) {
    for (uint32_t i=0; i < node->case_list.size(); i++) {
      auto pair = node->case_list[i];
      // pair.first == nullptr for default:
      symbolic_condition_t *sym_cond;
      if (pair.first) {
        const value_t c = walk_atom(pair.first);
        sym_cond = new symbolic_condition_t(new value_t(cond), new value_t(c),
            ExpressionOperation::EQ);

        switch (symbolic::check_condition(visitor.context_.path_conditions, sym_cond)) {
          case symbolic::check_status_t::NOT_FOUND: break;
          case symbolic::check_status_t::TRUE:
            symbolic::dump_pathcond_match(visitor.context_.trace, visitor.driver_.get_filename(),
                pair.first->location.begin.line, sym_cond, true);
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
              pair.first->location.begin.line, sym_cond);
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
void AstWalker<ExecutionVisitor, value_t>::walk_forall(ForallNode *node) {
  bool forked = false;
  const value_t in_list = walk_expression_base(node->in_expr);

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
      INT_T end =  in_list.value.integer;

      if (end > 0) {
        for (INT_T i = 0; i < end; i++) {
          visitor.rule_bindings.back()->push_back(value_t(i));
          walk_statement(node->statement);
          visitor.rule_bindings.back()->pop_back();
        }
      } else {
        for (INT_T i = 0; end < i; i--) {
          visitor.rule_bindings.back()->push_back(value_t(i));
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
          value_t v = value_t(pair.second);
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
void AstWalker<ExecutionVisitor, value_t>::walk_iterate(UnaryNode *node) {
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
void AstWalker<ExecutionVisitor, value_t>::walk_update(UpdateNode *node) {
  // this is used to dump %CREATE in trace if necessary
  const value_t &expr_t = walk_expression_base(node->expr_);
  if (visitor.context_.symbolic && node->func->symbol->is_symbolic) {
    walk_expression_base(node->func);
  }

  if (node->func->arguments) {
    uint16_t i;
    for (i=0; i < node->func->arguments->size(); i++) {
      visitor.arguments[i] = walk_expression_base(node->func->arguments->at(i));
    }
    visitor.num_arguments = i;
  } else {
    visitor.num_arguments = 0;
  }

  visitor.visit_update(node, expr_t);
}

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_update_subrange(UpdateNode *node) {
  const value_t &expr_t = walk_expression_base(node->expr_);

  // walk the function expression when argument list contains subrange types
  // to check the subranges and when function is symbolic to dump creates
  if (node->func->symbol->subrange_arguments.size() > 0 || (
      visitor.context_.symbolic && node->func->symbol->is_symbolic)) {
    walk_expression_base(node->func);
  }

  if (node->func->arguments) {
    uint16_t i;
    for (i=0; i < node->func->arguments->size(); i++) {
      visitor.arguments[i] = walk_expression_base(node->func->arguments->at(i));
    }
    visitor.num_arguments = i;
  } else {
    visitor.num_arguments = 0;
  }

  visitor.visit_update_subrange(node, expr_t);
}

template <>
void AstWalker<ExecutionVisitor, value_t>::walk_update_dumps(UpdateNode *node) {
  const value_t expr_t = walk_expression_base(node->expr_);

  if (node->func->arguments) {
    uint16_t i;
    for (i=0; i < node->func->arguments->size(); i++) {
      visitor.arguments[i] = walk_expression_base(node->func->arguments->at(i));
    }
    visitor.num_arguments = i;
  } else {
    visitor.num_arguments = 0;
  }

  visitor.visit_update_dumps(node, expr_t);
}

ExecutionWalker::ExecutionWalker(ExecutionVisitor& v) 
     : AstWalker<ExecutionVisitor, value_t>(v), initialized() {
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

  visitor.context_.function_states[func->id] = std::move(
      std::unordered_map<ArgumentsKey, value_t>(0, {func->arguments_}, {func->arguments_}));

  visitor.context_.function_symbols[func->id] = func;

  auto& function_map = visitor.context_.function_states[func->id];

  if (func->intitializers_ != nullptr) {
    for (std::pair<ExpressionBase*, ExpressionBase*> init : *func->intitializers_) {
      uint32_t num_arguments = 0;
      uint64_t *args = new uint64_t[10];
      if (init.first != nullptr) {
        value_t arguments[10];
        const value_t argument_v = walk_expression_base(init.first);
        if (func->arguments_.size() > 1) {
          List *list = argument_v.value.list;
          for (auto iter = list->begin(); iter != list->end(); iter++) {
            arguments[num_arguments] = *iter;
            num_arguments += 1;
          }
        } else {
            arguments[num_arguments] = argument_v;
            num_arguments += 1;
        }
        pack_values_in_array(arguments, &args[0], num_arguments);
      } else {
        args[0] = 0;
      }

      if (function_map.count(ArgumentsKey(&args[0], num_arguments, false, 0)) != 0) {
        yy::location loc = init.first ? init.first->location+init.second->location
                                      : init.second->location;
        visitor.driver_.error(loc, "function `"+func->name+"("+args_to_str(args, num_arguments)+")` already initialized");
        throw RuntimeException("function already initialized");
      }

      if (visitor.context_.symbolic && func->is_symbolic) {
        const value_t v = walk_expression_base(init.second);
        symbolic::dump_create(visitor.context_.trace_creates, func,
            &args[0], 0, v);
        function_map.emplace(std::pair<ArgumentsKey, value_t>(
              std::move(ArgumentsKey(&args[0], num_arguments, true, 0)), v));
      } else {
        value_t v = walk_expression_base(init.second);
        if (func->subrange_return) {
          if (v.value.integer < func->return_type_->subrange_start ||
            v.value.integer > func->return_type_->subrange_end) {
            yy::location loc = init.first ? init.first->location+init.second->location
                                          : init.second->location;
            visitor.driver_.error(loc,
                  std::to_string(v.value.integer)+" does violate the subrange "
                  +std::to_string(func->return_type_->subrange_start)
                  +".." +std::to_string(func->return_type_->subrange_end)
                  +" of `"+func->name+"`");
            throw RuntimeException("Subrange violated");
          }
        }
        function_map.emplace(std::pair<ArgumentsKey, value_t>(std::move(ArgumentsKey(&args[0], num_arguments, true, 0)), v));
      }
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
    const value_t program_val = visitor.context_.get_function_value(program_sym, args, 0);
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
    symbolic::dump_final(visitor.context_.trace, visitor.context_.function_symbols, visitor.context_.function_states);
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
