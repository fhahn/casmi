#include "libinterpreter/symbolic.h"

namespace symbolic {
  static uint32_t last_symbol_id = 0;

  uint32_t next_symbol_id() {
    last_symbol_id += 1;
    return last_symbol_id;
  }

  static uint32_t last_fof_id = 0;

  uint32_t next_fof_id() {
    last_fof_id += 1;
    return last_fof_id;
  }

  static uint32_t current_time = 1;
  void advance_timestamp() {
    current_time += 1;
  }

  uint32_t get_timestamp() {
    return current_time;
  }

  void dump_create_value(const std::string& name, const Value& v) {
    std::cout << "tff(symbolNext, type, sym" << v.value.ival<< ": $int)."
              << std::endl;
    std::cout << "fof(id"<<next_fof_id() << ",hypothesis,st" << name << "(1,sym"
              << v.value.ival<<")).%CREATE: " << name << std::endl;
  }

  void dump_update(const std::string& name, const Value& v) {
    std::cout << "fof(id" << next_fof_id() << ",hypothesis,st" << name 
              << "(" << get_timestamp() << ",sym" << v.value.ival
              << ")).%UPDATE: " << name << std::endl;
  }

  void dump_final(const std::vector<std::pair<Function*,
      std::unordered_map<ArgumentsKey, Value> >>& functions) {

    uint32_t i = 0;
    for (auto& pair : functions) {
      if (!pair.first->is_symbolic) {
        continue;
      }
      for (auto& value_pair : pair.second) {
       std::cout << "fof(final" << i << ",hypothesis,st" << pair.first->name
                 << "(0,sym" << value_pair.second.value.ival
                 << ")).%FINAL: " << pair.first->name << std::endl;
      }
      i += 1;
    }
  }
}
