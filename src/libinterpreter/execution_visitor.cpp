#include <assert.h>
#include <utility>

#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"


void pack_values_in_array(const std::vector<Value> &value_list, uint64_t array[]) {
  for (size_t i=0; i < value_list.size(); i++) {
    array[i] = value_list[i].to_uint64_t();
  }
}


ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, Driver& driver)
    : driver_(driver), context_(ctxt) {}

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

  // TODO initialize other fields
  up->value = (void*) expr_v.to_uint64_t();
  up->func = update->func->symbol->id;
  pack_values_in_array(value_list, up->args);

  up->num_args = value_list.size();

  value_list.clear();
  if(up->func == 0) {
    DEBUG("asd "<< value_list.size() << " asd "<<up->value);
  }
  casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                    (void*) update->func->symbol->id,
                                                    (void*) up);
  // TODO implement seq semantic
  if (v != nullptr) {
    driver_.error(update->func->location,
                  "Conflict in current block for function `"+update->func->name+"`");
    throw RuntimeException("Conflict in updateset");
  }
}

void ExecutionVisitor::visit_call_pre(CallNode *call) { UNUSED(call); }

void ExecutionVisitor::visit_call_pre(CallNode *call, Value& expr) {
  if (expr.type != Type::UNDEF) {
    call->rule = expr.value.rule;
  } else {
    throw RuntimeException("Cannot call UNDEF");
  }
}

void ExecutionVisitor::visit_call(CallNode *call, std::vector<Value> &argument_results) {
  UNUSED(call);
  UNUSED(argument_results);
}

void ExecutionVisitor::visit_print(PrintNode *node, const std::vector<Value> &arguments) {
  for (const Value& v: arguments) {
    std::cout << v.to_str();
  }
  std::cout << std::endl;
}

Value&& ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  switch (expr->op) {
    case Expression::Operation::ADD: {
      left_val.add(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::SUB: {
      left_val.sub(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::MUL: {
      left_val.mul(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::DIV: {
      left_val.div(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::MOD: {
      left_val.mod(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::RAT_DIV: {
      left_val.mod(right_val);
      return std::move(left_val);
    }

    case Expression::Operation::EQ: {
      left_val.eq(right_val);
      return std::move(left_val);
    }
    case Expression::Operation::NEQ: {
      left_val.eq(right_val);
      left_val.value.bval = !left_val.value.bval;
      return std::move(left_val);
    }

    case Expression::Operation::LESSER:
      left_val.lesser(right_val);
      return std::move(left_val);
    case Expression::Operation::GREATER:
      left_val.greater(right_val);
      return std::move(left_val);
    case Expression::Operation::LESSEREQ:
      left_val.lessereq(right_val);
      return std::move(left_val);
    case Expression::Operation::GREATEREQ:
      left_val.greatereq(right_val);
      return std::move(left_val);



    default: assert(0);
  }
}

Value&& ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  UNUSED(expr);
  return std::move(val);
}

Value&& ExecutionVisitor::visit_function_atom(FunctionAtom *atom, std::vector<Value> &expr_results) {
  size_t num_args = 1;
  if (expr_results.size() > 0) {
    num_args = expr_results.size();
  }

  uint64_t args[num_args];
  args[0] = 0;

  pack_values_in_array(expr_results, args);
  //
  // TODO handle function access and function write differently
  value_list.swap(expr_results);

  return std::move(Value(context_.get_function_value(atom->symbol, args)));
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

void ExecutionWalker::run() {
  for (auto pair: visitor.context_.symbol_table.table_) {
    auto function_map = std::unordered_map<ArgumentsKey, Value>();

    if (pair.second->intitializers_ != nullptr) {
      for (std::pair<ExpressionBase*, ExpressionBase*> init : *pair.second->intitializers_) {
        size_t num_args = 0; 
        uint64_t args[10];
        if (init.first != nullptr) {
          std::vector<Value> ident;
          ident.push_back(walk_expression_base(init.first));
          pack_values_in_array(ident, &args[0]);
          num_args = ident.size();
        } else {
          args[0] = 0;
        }

        if (function_map.count({&args[0], num_args}) != 0) {
          yy::location loc = init.first ? init.first->location+init.second->location : init.second->location;
          visitor.driver_.error(loc, "function `"+pair.first+"("+args_to_str(args, num_args)+")` already initialized");
          throw RuntimeException("function already initialized");
        }
        function_map.emplace(std::pair<ArgumentsKey, Value>({&args[0], num_args}, walk_expression_base(init.second)));
      }
    }
    visitor.context_.functions[pair.second->id] = std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>(pair.second, function_map);
  }

  Function *program_sym = visitor.context_.symbol_table.get("program");
  uint64_t args[10] = {0};
  while(true) {
    Value program_val = visitor.context_.get_function_value(program_sym, args);
    DEBUG(program_val.to_str());
    if (program_val.type == Type::UNDEF) {
      break;
    }
    walk_rule(program_val.value.rule);
    visitor.context_.apply_updates();
  }
}
