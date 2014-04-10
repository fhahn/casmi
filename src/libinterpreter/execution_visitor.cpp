#include <assert.h>

#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"

ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, RuleNode *init, Driver& driver)
    : driver_(driver), top_rule(init), context_(ctxt) {}

void ExecutionVisitor::visit_assert(UnaryNode* assert, Value& val) {
  if (val.value.bval != true) {
    driver_.error(assert->location,
                  "Assertion failed");
    throw RuntimeException("Assertion failed");
  }
}

void ExecutionVisitor::visit_update(UpdateNode *update, Value& func_val, Value& expr_v) {

  UNUSED(func_val);

  if (update->func->name == "program") {
    // TODO handle in a more efficient way
    top_rule = nullptr;
  } else {
    casm_update* up = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

    // TODO initialize other fields
    up->value = (void*) expr_v.value.ival;
    casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                      (void*) update->func->symbol->id,
                                                      (void*) up);
    // TODO implement seq semantic
    if (v != nullptr) {
      driver_.error(update->func->location,
                    "Conflict in current block for function `"+update->func->name+"`");
      throw RuntimeException("Conflict in updateset");
    }
    /*
    CASM_RT("S %lx", key);
    */
  }
}

Value&& ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  switch (expr->op) {
    case Expression::Operation::ADD: {
      switch (left_val.type) {
        case Type::INT: {
          left_val.value.ival += right_val.value.ival;
          return std::move(left_val);
        }
        default: assert(0);
      }
      break;
    }
    case Expression::Operation::EQ: {
      DEBUG ("got "<< left_val.value.ival << " and "<<right_val.value.ival);
      switch (left_val.type) {
        case Type::INT: {
          left_val.value.bval = left_val.value.ival == right_val.value.ival;
          return std::move(left_val);
        }
        case Type::BOOL: {
          left_val.value.bval = left_val.value.bval == right_val.value.bval;
          return std::move(left_val);
        }
        case Type::UNDEF:
          if (right_val.type == Type::UNDEF) {
            left_val.value.bval = true;
          } else {
            left_val.value.bval = false;
          }
          return std::move(left_val);
        default: assert(0);
      }
    }
    default: assert(0);
  }
}

Value&& ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  UNUSED(expr);
  return std::move(val);
}

Value&& ExecutionVisitor::visit_function_atom(FunctionAtom *atom, const std::vector<Value> &expr_results) {
  casm_update *data = context_.get_function_value(atom->symbol);
  if (data == nullptr) {
    return std::move(Value());
  }

  switch (atom->symbol->return_type_) {
    case Type::INT: return std::move(Value((int64_t)data->value));
    default: throw "invalid type in function";
  }

}

void ExecutionWalker::run() {
  while (visitor.top_rule != nullptr) {
    walk_rule(visitor.top_rule);
    visitor.context_.apply_updates();
  }
}
