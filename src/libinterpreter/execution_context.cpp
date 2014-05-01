#include "libutil/exceptions.h"

#include "librt/pp_hashmap.h"

#include "libinterpreter/execution_context.h"

ExecutionContext::ExecutionContext(const SymbolTable<Function*>& st, RuleNode *init) : symbol_table(std::move(st)), temp_lists() {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 100, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024*10, "main updateset");

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024 * 100, "mem for stack stuff");

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

  std::vector<Value*> to_fold;
  while( i != updateset.set->head ) {
    u = (casm_update*)i->value;

    CASM_RT("update function %lu, %p = %p, %u", u->func, u, (void*)u->value, u->defined);

    auto& function_map = functions[u->func];

    DEBUG("APPLY args "<<u->num_args << " arg "<<u->args[0] << " " << u->args[1]<<" func "<<function_map.first->name());

    if (function_map.first->return_type_->t == TypeType::LIST) {
      Value& list = function_map.second[{u->args, u->num_args}];
      if (u->defined == 0) {

        // TODO HANDLE overwriting lists with undef
        if (list.is_undef()) {
          function_map.second.erase({u->args, u->num_args});
        }
      } else {
        list.type = function_map.first->return_type_->t;
        // TODO HANDLE overwriting old lists
        //list.value.list->decrease_usage();
        list.value.list = reinterpret_cast<List*>(u->value);
        list.value.list->bump_usage();
        to_fold.push_back(&list);
      }
    } else {
      Value v(function_map.first->return_type_, u);
      if (v.type == TypeType::UNDEF) {
        function_map.second.erase({u->args, u->num_args});
      } else {
        function_map.second[{u->args, u->num_args}] = v;
      }
    }

    i->used = 0;
    i = i->previous;
  }

  /*
  for (Value* v : to_fold) {
    std::vector<Value> vals;
    v->value.list = new BottomList(v->value.list->collect(vals));
  }
  to_fold.clear(); 
  std::vector<size_t> deleted;

  for (size_t i=0; i < temp_lists.size(); i++) {
    if (!temp_lists[i]->is_used()) {
      delete temp_lists[i];
      deleted.push_back(i);
    }
  }

  for (size_t del : deleted) {
    temp_lists[del] = std::move(temp_lists.back());
    temp_lists.pop_back();
  }
  */

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

bool args_eq(uint64_t args1[], uint64_t args2[], size_t len) {

  DEBUG("LEN "<<len);
  for (size_t i=0; i < len; i++) {
    DEBUG("args1 "<<args1[i]<<"args2 "<<args2[i]);
    if (args1[i] != args2[i]) {
      return false;
    }
  }
  return true;
}

Value& ExecutionContext::get_function_value(Function *sym, uint64_t args[]) {
  // TODO move should be used here
  auto& function_map = functions[sym->id];
  try {
      DEBUG("get "<<sym->id << " " << sym->name()<<" size:"<<sym->arguments_.size() << " args "<<args[0] << " Fun size "<< function_map.second.size());
    Value &v = function_map.second.at({&args[0], sym->arguments_.size()});
    int64_t state = (updateset.pseudostate % 2 == 0) ? state = updateset.pseudostate-1:
                                                       state = updateset.pseudostate;
    for (; state > 0; state -= 2) {
      uint64_t key = (uint64_t) &v << 16 | state;
      casm_update *update = (casm_update*) pp_hashmap_get(updateset.set, key);
      if (update) {
        tmp = Value(sym->return_type_, update);
        DEBUG("FOUND UPDATE for "<< sym->name()<<" "<<tmp.to_str() << " type "<<sym->return_type_->to_str());
        return tmp;
      }
    }
    DEBUG("FOUNDDDD");
    return v;

  } catch (const std::out_of_range &e) {
    DEBUG("NOT FOUND");
    undef.type = TypeType::UNDEF;
    return undef;
  }
}
