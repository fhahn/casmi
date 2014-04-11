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

Value&& ExecutionVisitor::visit_function_atom(FunctionAtom *atom, std::vector<Value> &expr_results) {
  size_t num_args = 1;
  if (expr_results.size() > 0) {
    num_args = expr_results.size();
  
  }
  uint64_t args[num_args];
  args[0] = 0;

  pack_values_in_array(value_list, args);
  casm_update *data = context_.get_function_value(atom->symbol, args);
  DEBUG("foo "<< value_list.size() << " asd "<<args[0] << " func " << atom->symbol->id);

  // TODO handle function access and function write differently
  value_list.swap(expr_results);

  if (data == nullptr) {
    return std::move(Value());
  }

  switch (atom->symbol->return_type_) {
    case Type::INT: return std::move(Value((int64_t)data->value));
    case Type::RULEREF: return std::move(Value(reinterpret_cast<RuleNode*>(data->value)));
    default: throw "invalid type in function";
  }

}

void ExecutionWalker::run() {
  for (auto pair: visitor.context_.symbol_table->table_) {
    auto function_map = std::unordered_map<ArgumentsKey, casm_update*>();

    if (pair.second->intitializers_ != nullptr) {
      for (std::pair<AtomNode*, AtomNode*> init : *pair.second->intitializers_) {
        casm_update* up = (casm_update*) pp_mem_alloc(&visitor.context_.pp_stack, sizeof(casm_update));

        size_t num_args = 0; 
        if (init.first != nullptr) {
          std::vector<Value> ident;
          ident.push_back(walk_atom(init.first));
          pack_values_in_array(ident, &up->args[0]);
          num_args = ident.size();
        }

        up->func = pair.second->id;
        up->value = (void*) walk_atom(init.second).to_uint64_t();
        DEBUG("init value" << up->value << " func " << up->func << " num arg "<<num_args);
        up->args[0] = 0;
        // TODO implement for functions with arguments
        function_map[{&up->args[0], num_args}] = up;
      }
    }
    visitor.context_.functions.push_back(std::move(function_map));
  }


  Symbol *program_sym = visitor.context_.symbol_table->get("program");
  uint64_t args[1] = {0};
  while(true) {
    casm_update *program_val = visitor.context_.get_function_value(program_sym, args);
    if (program_val->value == 0) {
      break;
    }
    walk_rule(reinterpret_cast<RuleNode*>(program_val->value));
    visitor.context_.apply_updates();
  }
}
