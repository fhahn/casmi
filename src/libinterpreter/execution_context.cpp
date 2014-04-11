#include "librt/pp_hashmap.h"

#include "libinterpreter/execution_context.h"

ExecutionContext::ExecutionContext(SymbolTable *st, RuleNode *init) : symbol_table(st) {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 10, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024, "main updateset");
  updateset.pseudostate = 0;

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024 * 10, "mem for stack stuff");

  pseudostate = 0;

  Symbol *program_sym = st->get("program");
  auto program_map = functions[program_sym->id];
  casm_update* up = (casm_update*) pp_mem_alloc(&pp_stack, sizeof(casm_update));
  up->value = (void*) init;
  up->args[0] = 0;
  program_map[{&up->args[0], 1}] = up;
  functions[program_sym->id] = program_map;
}

void ExecutionContext::apply_updates() {
  //CASM_RT("updateset apply(updateset = %p)", &updateset);

  pp_hashmap_bucket* i = updateset.set->tail->previous;
  casm_update* u;

  while( i != updateset.set->head ) {
    u = (casm_update*)i->value;

    CASM_RT("update function %lu, %p = %p, %u", u->func, u, (void*)u->value, u->defined);

    auto function_map = functions[u->func];

    DEBUG("APPLY args "<<u->num_args << " argi "<<u->args[0]<<" func ");
    function_map[{u->args, u->num_args}] = u;
    functions[u->func] = function_map;

    i->used  = FALSE;
    i = i->previous;
  }

  updateset.set->head->previous = NULL;
  updateset.set->head->next     = updateset.set->tail;

  updateset.set->tail->previous = updateset.set->head;
  updateset.set->tail->next     = NULL;

  updateset.set->count = 0;
}


void ExecutionContext::set_function(Symbol *sym, casm_update *update) {
  auto function_map = functions[sym->id];
  function_map[{&update->args[0], sym->argument_count()}] = update;
}

casm_update* ExecutionContext::get_function_value(Symbol *sym, uint64_t args[]) {
  auto function_map = functions[sym->id];
  try {
    if (sym->arguments_) {
      return function_map.at({&args[0], sym->arguments_->size()});
    } else {
      return function_map.at({&args[0], 0});
    }
  } catch (const std::out_of_range &e) {
    return nullptr;
  }
}
