#include "libutil/exceptions.h"

#include "librt/pp_hashmap.h"

#include "libinterpreter/execution_context.h"

ExecutionContext::ExecutionContext(const SymbolTable<Function*>& st, RuleNode *init) : symbol_table(std::move(st)) {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 10, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024, "main updateset");

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024 * 10, "mem for stack stuff");

  if (init->child_ && init->child_->node_type_ == NodeType::PARBLOCK) {
    updateset.pseudostate = 1;
  } else {
    updateset.pseudostate = 0;
  }

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

    i->used = 0;
    i = i->previous;
  }

  updateset.set->head->previous = NULL;
  updateset.set->head->next     = updateset.set->tail;

  updateset.set->tail->previous = updateset.set->head;
  updateset.set->tail->next     = NULL;

  updateset.set->count = 0;
}

void ExecutionContext::merge_par() {
        /*pp_measure_start(&updateset->time_merge[CASM_MODE_PAR]);*/

        //CASM_RT("updateset merge par");

        /*casm_updateset_print_debug(updateset, "pre-merge");
         */

        updateset.pseudostate--;

        //CASM_RT("merge-par:");

        pp_hashmap_bucket* j = updateset.set->tail->previous;
        pp_hashmap_bucket* i;

        while( j != updateset.set->head ) {
            i = j;
            j = j->previous;

            if( (uint16_t)i->key <= updateset.pseudostate ) break;

            pp_hashmap_delete(updateset.set, i);

            pp_hashmap_set(updateset.set, i->key-1, i->value);

            // CASM_RT("%p: %p @ %lx --> %lx", i, i->value, i->key, i->key-1);

            /*i = i->previous;*/
        }

        //CASM_RT("merge-END");

        /* pp_measure_stop(&updateset->time_merge[CASM_MODE_PAR]); */

        /*casm_updateset_print_debug(updateset, "post-merge");*/
}

void ExecutionContext::merge_seq(Driver& driver) {
        /*pp_measure_start(&updateset->time_merge[CASM_MODE_SEQ]);*/

        //CASM_RT("updateset merge seq");

        /*casm_updateset_print_debug(updateset, "pre-merge");
         */

        updateset.pseudostate--;

        //CASM_RT("merge-seq");

        pp_hashmap_bucket* j = updateset.set->tail->previous;
        pp_hashmap_bucket* i;
        casm_update* u;
        casm_update* v;

        while( j != updateset.set->head ) {
            i = j;
            j = j->previous;

            //CASM_RT("%p: %p @ %lx ...", i, i->value, i->key);

            if( (uint16_t)i->key <= updateset.pseudostate ) break;

            pp_hashmap_delete(updateset.set, i);

            if( (v = (casm_update*) pp_hashmap_set(updateset.set, i->key-1, i->value)) != NULL ) {
                u = (casm_update*)i->value;

                UpdateNode *up = reinterpret_cast<UpdateNode*>(u->line);
                if (up->func->arguments) {
                  for (size_t i=0; i < up->func->arguments->size(); i++) {
                    if (u->args[i] != v->args[i]) {
                      return;
                    }
                  }
                }

                driver.error(up->location, "conflict merging updatesets");
                throw RuntimeException("merge error");
            }

            //CASM_RT("%p: %p @ %lx --> %lx", i, i->value, i->key, i->key-1);

            /*i = i->previous;*/
        }

        //CASM_RT("merge-END");

        /*pp_measure_stop(&updateset->time_merge[CASM_MODE_SEQ]); */

        /*casm_updateset_print_debug(updateset, "post-merge");*/
    }



void ExecutionContext::set_function(Function *sym, uint64_t args[], Value& val) {
  auto function_map = functions[sym->id];
  function_map.second.insert(std::pair<ArgumentsKey, Value>({&args[0], sym->argument_count()}, val));
}

static Value undef = Value();

static Value tmp;

Value& ExecutionContext::get_function_value(Function *sym, uint64_t args[]) {
  // TODO move should be used here
  int64_t state = (updateset.pseudostate % 2 == 0) ? state = updateset.pseudostate-1:
  state = updateset.pseudostate;
  for (; state > 0; state -= 2) {
    uint64_t key = (uint64_t) sym->id << 16 | state;
    casm_update *update = (casm_update*) pp_hashmap_get(updateset.set, key);
    if (update) {
      tmp = Value(sym->return_type_, update);
      return tmp;
    }
  }

  auto& function_map = functions[sym->id];
  try {
      DEBUG("get "<<sym->id << " " << sym->name()<<" size:"<<sym->arguments_.size() << " args "<<args[0]);
    return function_map.second.at({&args[0], sym->arguments_.size()});
  } catch (const std::out_of_range &e) {
    undef.type = Type::UNDEF;
    return undef;
  }
}
