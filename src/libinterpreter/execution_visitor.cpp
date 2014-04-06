#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_visitor.h"

ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, RuleNode *init, Driver& driver)
    : context_(ctxt), driver_(driver), top_rule(init) {}

void ExecutionVisitor::visit_update(UpdateNode *update, Value& val) {

  if (update->sym_->symbol->name() == "program") {
    // TODO handle in a more efficient way
    top_rule = nullptr;
  } else {
    uint64_t key = (uint64_t) update->sym_->symbol << 16 | (uint64_t)context_.pseudostate;
    casm_update* up = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

    // TODO initialize other fields
    up->value = (void*) val.value.ival;
    casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                      update->sym_->symbol,
                                                      (void*) up);
    // TODO implement seq semantic
    if (v != nullptr) {
      driver_.error(update->sym_->location,
                    "Conflict in current block for function `"+update->sym_->name_+"`");
      throw RuntimeException("Conflict in updateset");
    }
    /*
    CASM_RT("S %lx", key);
    up = (casm_update*) pp_hashmap_get( context_.updateset.set, key );
    */
  }
}

Value&& ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  switch (left_val.type) {
    case Type::INT: {
      left_val.value.ival += right_val.value.ival;
      return std::move(left_val);
    }
    default: {
      throw std::string("KABOOM");
    }
  }
}

Value&& ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  return std::move(val);
}

void ExecutionWalker::run() {
  while (visitor.top_rule != nullptr) {
    walk_rule(visitor.top_rule);
  }
}
