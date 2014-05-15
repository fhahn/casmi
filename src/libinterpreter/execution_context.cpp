#include "libutil/exceptions.h"

#include "libinterpreter/execution_context.h"

ArgumentsKey::ArgumentsKey(uint64_t *args, uint16_t s, bool dyn) : size(s), dynamic(dyn){
  if (dynamic) {
    p = new uint64_t[size];
    for (uint16_t i = 0; i < size; i++) {
      p[i] = args[i];
    }
  } else {
    p = args;
  }
}

ArgumentsKey::ArgumentsKey(const ArgumentsKey& other) : p(other.p), size(other.size), dynamic(other.dynamic) {
}

ArgumentsKey::ArgumentsKey(ArgumentsKey&& other) noexcept {
  p = other.p;
  size = other.size;
  dynamic = other.dynamic;
  other.dynamic = false;
}

ArgumentsKey::~ArgumentsKey() {
  if (dynamic) {
    delete[] p;
  }
}

pp_mem ExecutionContext::value_stack;

ExecutionContext::ExecutionContext(const SymbolTable& st, RuleNode *init) : debuginfo_filters(), symbol_table(std::move(st)), temp_lists() {
  // use 10 MB for updateset data
  pp_mem_new(&updateset_data_, 1024 * 1024 * 100, "mem for main updateset");
  updateset.set =  pp_hashmap_new(&updateset_data_, 1024*10, "main updateset");

  // use 10 MB for stack
  pp_mem_new(&pp_stack, 1024 * 1024, "mem for stack stuff");
  pp_mem_new(&value_stack, 1024 * 1024 * 1024, "mem for value stuff");

  if (init->child_ && init->child_->node_type_ == NodeType::PARBLOCK) {
    updateset.pseudostate = 1;
  } else {
    updateset.pseudostate = 0;
  }

  functions = std::vector<std::pair<Function*, std::unordered_map<ArgumentsKey, Value>>>(symbol_table.size());
  Function *program_sym = symbol_table.get_function("program");
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

    auto& function_map = functions[u->func];

    DEBUG("APPLY args "<<u->num_args << " arg "<<u->args[0] << " " << u->args[1]<<" func "<<function_map.first->name);

    // TODO handle tuples
    if (function_map.first->return_type_->t == TypeType::LIST) {
      Value& list = function_map.second[ArgumentsKey(u->args, u->num_args, false)];
      if (u->defined == 0) {
        // set list to undef
        if (!list.is_undef()) {
          list.value.list->decrease_usage();
          list.type = TypeType::UNDEF;
        }
      } else {
        if (!list.is_undef()) {
          list.value.list->decrease_usage();
        } else {
          list.type = function_map.first->return_type_->t;
        }
        list.value.list = reinterpret_cast<List*>(u->value);
        list.value.list->bump_usage();
        to_fold.push_back(&list);
      }
    } else {
      Value v(function_map.first->return_type_->t, u);
      if (v.type == TypeType::UNDEF) {
        function_map.second.erase(ArgumentsKey(u->args, u->num_args, false));
      } else {
        function_map.second[ArgumentsKey(u->args, u->num_args, true)] = v;
      }
    }

    i->used = 0;
    i = i->previous;
  }

  for (Value* v : to_fold) {
    BottomList *new_l = v->value.list->collect();
    if (new_l->check_allocated_and_set_to_false()) {
      temp_lists.push_back(new_l);
    }
    v->value.list = new_l;
  }
  to_fold.clear(); 
  std::vector<size_t> deleted;

  for (size_t i=0; i < temp_lists.size(); i++) {
    // delete all list objects, except BottomLists that are currently used
    if (!(temp_lists[i]->is_bottom() && reinterpret_cast<BottomList*>(temp_lists[i])->is_used())) {
      delete temp_lists[i];
      deleted.push_back(i);
    }
  }

  for (size_t del : deleted) {
    temp_lists[del] = std::move(temp_lists.back());
    temp_lists.pop_back();
  }


  // free allocated updateset data
  pp_mem_free(&updateset_data_);
  pp_mem_free(&pp_stack);

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

                for (size_t i=0; i < u->num_args; i++) {
                  if (u->args[i] != v->args[i]) {
                    return;
                  }
                }
                driver.error(*reinterpret_cast<yy::location*>(u->line), "conflict merging updatesets");
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
  function_map.second.insert(std::pair<ArgumentsKey, Value>(ArgumentsKey(&args[0], sym->argument_count(), true), val));
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
      DEBUG("get "<<sym->id << " " << sym->name<<" size:"<<sym->arguments_.size() << " args "<<args[0] << " Fun size "<< function_map.second.size());
    Value &v = function_map.second.at(ArgumentsKey(&args[0], sym->arguments_.size(), false));
    int64_t state = (updateset.pseudostate % 2 == 0) ? state = updateset.pseudostate-1:
                                                       state = updateset.pseudostate;
    for (; state > 0; state -= 2) {
      uint64_t key = (uint64_t) &v << 16 | state;
      casm_update *update = (casm_update*) pp_hashmap_get(updateset.set, key);
      if (update) {
        tmp = Value(sym->return_type_->t, update);
        DEBUG("FOUND UPDATE for "<< sym->name<<" "<<tmp.to_str() << " type "<<sym->return_type_->to_str());
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

bool ExecutionContext::set_debuginfo_filter(const std::string& filters) {
  std::string current;
  size_t last_pos = 0;
  size_t match_pos = 0;

  while ((match_pos = filters.find(",", last_pos)) != std::string::npos) {
    current = filters.substr(last_pos, match_pos-last_pos);
    if (current.size() > 0) {
      debuginfo_filters[current] = true;
    }
    last_pos = match_pos+1;
  }

  match_pos = filters.size();
  current = filters.substr(last_pos, match_pos-last_pos);
  if (current.size() > 0) {
    debuginfo_filters[current] = true;
  }

  return true;
}

bool ExecutionContext::filter_enabled(const std::string& filter) {
  return debuginfo_filters.count("all") > 0 || debuginfo_filters.count(filter) > 0;
}
