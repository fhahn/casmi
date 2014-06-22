#include "macros.h"
#include "libutil/exceptions.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/symbolic.h"

#define UPDATESET_DATA_SIZE 10 * 1024 * 1024
#define UPDATESET_SIZE 10 * 1024
#define TEMP_STACK_SIZE 1024 * 1024


ArgumentsKey::ArgumentsKey(uint64_t *args, uint16_t size, bool dyn,
    uint16_t syms) : dynamic(dyn), sym_args(syms) {
  if (dynamic) {
    p = new uint64_t[size];
    for (uint16_t i = 0; i < size; i++) {
      p[i] = args[i];
    }
  } else {
    p = args;
  }
}

ArgumentsKey::ArgumentsKey(const ArgumentsKey& other) : p(other.p),
    dynamic(other.dynamic), sym_args(other.sym_args) {
}

ArgumentsKey::ArgumentsKey(ArgumentsKey&& other) noexcept {
  p = other.p;
  dynamic = other.dynamic;
  other.dynamic = false;
  sym_args = other.sym_args;
}

ArgumentsKey::~ArgumentsKey() {
  if (dynamic) {
    delete[] p;
  }
}

pp_mem ExecutionContext::value_stack;

ExecutionContext::ExecutionContext(const SymbolTable& st, RuleNode *init,
    const bool symbolic, const bool fileout): debuginfo_filters(),
    symbol_table(std::move(st)), temp_lists(), symbolic(symbolic), fileout(fileout),
    trace_creates(), trace(), path_name(""), path_conditions(), update_counter(0) {

  pp_mem_new(&updateset_data_, UPDATESET_DATA_SIZE, "mem for updateset hashmap");
  updateset.set =  pp_hashmap_new(&updateset_data_, UPDATESET_SIZE, "main updateset");

  pp_mem_new(&pp_stack, TEMP_STACK_SIZE, "mem for temporary updates");
  pp_mem_new(&value_stack, 1024 * 1024 * 1024, "mem for value stuff");

  if (init->child_ && init->child_->node_type_ == NodeType::PARBLOCK) {
    updateset.pseudostate = 1;
  } else {
    updateset.pseudostate = 0;
  }

  functions = std::vector<std::pair<const Function*, std::unordered_map<ArgumentsKey, value_t>>>(symbol_table.size());
  Function *program_sym = symbol_table.get_function("program");
  // TODO location is wrong here
  program_sym->intitializers_ = new std::vector<std::pair<ExpressionBase*, ExpressionBase*>>();
  RuleAtom *init_atom = new RuleAtom(init->location, std::string(init->name));
  init_atom->rule = init;
  program_sym->intitializers_->push_back(std::pair<ExpressionBase*, ExpressionBase*>(new SelfAtom(init->location), init_atom));

}

ExecutionContext::ExecutionContext(const ExecutionContext& other) : 
     debuginfo_filters(other.debuginfo_filters), symbol_table(other.symbol_table),
     symbolic(other.symbolic), fileout(other.fileout), trace(other.trace),
     path_name(other.path_name) {

  // TODO copy updates!
  pp_mem_new(&updateset_data_, UPDATESET_DATA_SIZE, "mem for updateset hashmap");
  updateset.set =  pp_hashmap_new(&updateset_data_, UPDATESET_SIZE, "main updateset");

  pp_mem_new(&pp_stack, TEMP_STACK_SIZE, "mem for temporary updates");

}

void ExecutionContext::apply_updates() {
  pp_hashmap_bucket* i = updateset.set->tail->previous;
  casm_update* u;

  std::unordered_map<uint32_t, std::vector<ArgumentsKey>> updated_functions;
  if (symbolic) {
    for (uint32_t i = 1; i < functions.size(); i++) {
      updated_functions[i] = std::vector<ArgumentsKey>();
    }
  }

  std::vector<value_t*> to_fold;
  while( i != updateset.set->head ) {
    u = (casm_update*)i->value;

    auto& function_map = functions[u->func];

    // TODO handle tuples
    if (function_map.first->return_type_->t == TypeType::LIST) {
      value_t& list = function_map.second[ArgumentsKey(u->args, u->num_args, false, u->sym_args)];
      if (u->symbolic){
        value_t v(function_map.first->return_type_->t, u);
        function_map.second[ArgumentsKey(u->args, u->num_args, true, u->sym_args)] = v;
      } else if (u->defined == 0) {
        // set list to undef
        if (!list.is_undef()) {
          list.value.list->decrease_usage();
          list.type = TypeType::UNDEF;
        }
      } else {
        if (!list.is_undef() && !list.is_symbolic()) {
          list.value.list->decrease_usage();
        } else {
          list.type = function_map.first->return_type_->t;
        }
        list.value.list = reinterpret_cast<List*>(u->value);
        list.value.list->bump_usage();
        to_fold.push_back(&list);
      }
    } else {
      value_t v(function_map.first->return_type_->t, u);
      // we could erase keys that store an undef value in concrete mode,
      // but we need to know if a key was set to undef explicitly in symbolic
      // mode
      function_map.second[ArgumentsKey(u->args, u->num_args, true, u->sym_args)] = v;
    }

    if (symbolic) {
      updated_functions[u->func].push_back(
            ArgumentsKey(u->args, u->num_args, true, u->sym_args));
    }

    i->used = 0;
    i = i->previous;
  }

  if (symbolic) {
    for (uint32_t i = 1; i < functions.size(); i++) {
      auto& function_map = functions[i];
      const auto& updated_keys = updated_functions[i];
      if (!function_map.first->is_symbolic || function_map.first->is_static) {
        continue;
      }

      std::equal_to<ArgumentsKey> eq = {function_map.first->arguments_};
      for (const auto& pair : function_map.second) {
        bool found = false;
        for (const auto& k : updated_keys) {
          if (eq(k, pair.first)) {
            found = true;
            break;
          }
        }
        if (!found) {
          symbolic::dump_symbolic(trace, function_map.first, pair.first.p,
              pair.first.sym_args, pair.second);
        }
      }
      for (const auto& k : updated_keys) {
        symbolic::dump_update(trace, function_map.first, k.p,
         k.sym_args, function_map.second[k]);
      }
    }
  }
  for (value_t* v : to_fold) {
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
  updateset.pseudostate--;

  pp_hashmap_bucket* j = updateset.set->tail->previous;
  pp_hashmap_bucket* i;

  while( j != updateset.set->head ) {
      i = j;
      j = j->previous;

      if( (uint16_t)i->key <= updateset.pseudostate ) break;

      pp_hashmap_delete(updateset.set, i);

      pp_hashmap_set(updateset.set, i->key-1, i->value);

  }

}

void ExecutionContext::merge_seq(Driver& driver) {
  updateset.pseudostate--;

  pp_hashmap_bucket* j = updateset.set->tail->previous;
  pp_hashmap_bucket* i;
  casm_update* u;
  casm_update* v;

  while( j != updateset.set->head ) {
      i = j;
      j = j->previous;

      if( (uint16_t)i->key <= updateset.pseudostate ) break;

      pp_hashmap_delete(updateset.set, i);

      if( (v = (casm_update*) pp_hashmap_set(updateset.set, i->key-1, i->value)) != NULL ) {
          u = (casm_update*)i->value;

          const Function *func = functions[u->func].first;
          for (size_t i=0; i < func->arguments_.size(); i++) {
            if (!eq_uint64_value(func->arguments_[i], u->args[i], v->args[i])) {
              return;
            }
          }
          driver.error(*reinterpret_cast<yy::location*>(u->line), "conflict merging updatesets");
          throw RuntimeException("merge error");
      }
  }
}

static value_t undef = value_t();

static value_t tmp;

bool args_eq(uint64_t args1[], uint64_t args2[], size_t len) {

  for (size_t i=0; i < len; i++) {
    if (args1[i] != args2[i]) {
      return false;
    }
  }
  return true;
}

const value_t ExecutionContext::get_function_value(Function *sym, uint64_t args[], uint16_t sym_args) {
  // TODO move should be used here
  auto& function_map = functions[sym->id];
  try {
    value_t &v = function_map.second.at(ArgumentsKey(&args[0], sym->arguments_.size(), false, sym_args));
    int64_t state = (updateset.pseudostate % 2 == 0) ? updateset.pseudostate-1:
                                                       updateset.pseudostate;
    for (; state > 0; state -= 2) {
      uint64_t key = (uint64_t) &v << 16 | state;
      casm_update *update = (casm_update*) pp_hashmap_get(updateset.set, key);
      if (update) {
        return value_t(sym->return_type_->t, update);
      }
    }
    return v;

  } catch (const std::out_of_range &e) {
    if (symbolic && sym->is_symbolic) {
      // TODO cleanup symbol
      function_map.second.emplace(
          ArgumentsKey(&args[0], sym->arguments_.size(), true, sym_args),
          value_t(new symbol_t(symbolic::next_symbol_id())));
      value_t& v = function_map.second[ArgumentsKey(&args[0], sym->arguments_.size(), false, sym_args)];
      symbolic::dump_create(trace_creates, sym, &args[0], sym_args, v);
      return v;
    }
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
