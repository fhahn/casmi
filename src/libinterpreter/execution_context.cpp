#include "librt/pp_hashmap.h"

#include "libinterpreter/execution_context.h"

ExecutionContext::ExecutionContext(SymbolTable *st) : current_st_(st) {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 10, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024, "main updateset");
  updateset.pseudostate = 0;

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024 * 10, "mem for stack stuff");

  pseudostate = 0;

  for (auto pair: st->table_) {
    functions.push_back(std::unordered_map<ArgumentsKey, casm_update*>());
  }
}

void ExecutionContext::apply_updates() {
  //CASM_RT("updateset apply(updateset = %p)", &updateset);

  pp_hashmap_bucket* i = updateset.set->tail->previous;
  casm_update* u;

  while( i != updateset.set->head ) {
    u = (casm_update*)i->value;

    CASM_RT("update function %u, %p = %p, %u", u->func, u, (void*)u->value, u->defined);

    auto function_map = functions[u->func];
    
    function_map[{&u->args[0], 0}] = u;

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
