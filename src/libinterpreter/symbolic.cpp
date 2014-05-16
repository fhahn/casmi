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
    std::cout << "tff(symbolNext, type, sym" << v.value.symbol << ": $int)."
              << std::endl;
    std::cout << "fof(id"<<next_fof_id() << ",hypothesis,st" << name << "(1,sym"
              << v.value.symbol<<")).%CREATE: " << name << std::endl;
  }
}
