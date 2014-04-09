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
