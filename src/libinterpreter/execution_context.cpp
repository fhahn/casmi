#include "libutil/exceptions.h"

#include "librt/pp_hashmap.h"

#include "libinterpreter/execution_context.h"

ExecutionContext::ExecutionContext(const SymbolTable<Function*>& st, RuleNode *init) : symbol_table(std::move(st)) {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 10, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024, "main updateset");
  updateset.pseudostate = 0;

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024 * 10, "mem for stack stuff");

  pseudostate = 0;

  functions = std::vector<std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>>(symbol_table.size());
  Function *program_sym = symbol_table.get("program");
  // TODO location is wrong here
  program_sym->intitializers_ = new std::vector<std::pair<ExpressionBase*, ExpressionBase*>>();
  RuleAtom *init_atom = new RuleAtom(init->location, std::string(init->name));
  init_atom->rule = init;
  program_sym->intitializers_->push_back(std::pair<ExpressionBase*, ExpressionBase*>(new SelfAtom(init->location), init_atom));
}

void ExecutionContext::apply_updates() {
  //CASM_RT("updateset apply(updateset = %p)", &updateset);

  pp_hashmap_bucket* i = updateset.set->tail->previous;
  casm_update* u;

  while( i != updateset.set->head ) {
    u = (casm_update*)i->value;

    CASM_RT("update function %lu, %p = %p, %u", u->func, u, (void*)u->value, u->defined);

    auto& function_map = functions[u->func];

    DEBUG("APPLY args "<<u->num_args << " arg "<<u->args[0] << " " << u->args[1]<<" func "<<u->func);

    Value v(function_map.first->return_type_, u);
    if (v.type == Type::UNDEF) {
      function_map.second.erase({u->args, u->num_args});
    } else {
      function_map.second[{u->args, u->num_args}] = v;
    }

    i->used = FALSE;
    i = i->previous;
  }

  updateset.set->head->previous = NULL;
  updateset.set->head->next     = updateset.set->tail;

  updateset.set->tail->previous = updateset.set->head;
  updateset.set->tail->next     = NULL;

  updateset.set->count = 0;
}


void ExecutionContext::set_function(Function *sym, uint64_t args[], Value& val) {
  auto function_map = functions[sym->id];
  function_map.second.insert(std::pair<ArgumentsKey, Value>({&args[0], sym->argument_count()}, val));
}

static Value undef = Value();

Value& ExecutionContext::get_function_value(Function *sym, uint64_t args[]) {
  auto& function_map = functions[sym->id];
  try {
    if (sym->arguments_) {
      DEBUG("get "<<sym->id << " " << sym->name()<<" size:"<<sym->arguments_->size() << " args "<<args[0]);
      return function_map.second.at({&args[0], sym->arguments_->size()});
    } else {
      return function_map.second.at({&args[0], 0});
    }
  } catch (const std::out_of_range &e) {
    undef.type = Type::UNDEF;
    return undef;
  }
}
