#include "macros.h"

#include "libinterpreter/execution_visitor.h"

ExecutionVisitor::ExecutionVisitor(ExecutionContext &ctxt, RuleNode *init)
    : context_(ctxt), top_rule(init) {}

void ExecutionVisitor::visit_update(UpdateNode *update, Value& val) {

  if (update->sym_->symbol->name() == "program") {
    // TODO handle in a more efficient way
    top_rule = nullptr;
  } else {

    casm_update* u = (casm_update*) pp_mem_alloc(&(context_.pp_stack), sizeof(casm_update));

    // TODO initialize other fields
    u->value = (void*) val.value.ival;

    casm_update* v = (casm_update*)casm_updateset_add(&(context_.updateset),
                                                      update->sym_->symbol,
                                                      (void*) u);

    int64_t ps = 0;

    uint64_t key = (uint64_t) update->sym_->symbol << 16 | (uint64_t)ps;
    casm_update* us;


    CASM_RT("S %lx", key);
    us = (casm_update*) pp_hashmap_get( context_.updateset.set, key );

    std::cout << "asd " << us << "\n";
    if( us != NULL )
    { 
      std::cout << "Got value: " << (uint64_t) us->value << "\n";
    }
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
