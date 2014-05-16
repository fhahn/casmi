#ifndef CASMI_LIBINTERPRETER_SYMBOLIC_H
#define CASMI_LIBINTERPRETER_SYMBOLIC_H

#include <cstdint>

#include "libinterpreter/value.h"

namespace symbolic {
  uint32_t next_symbol_id();
  uint32_t next_fof_id();
  void advance_timestamp();
  uint32_t get_timestamp();


  void dump_create_value(const std::string& name, const Value& v);
  void dump_update(const std::string& name, const Value& v);
};

#endif
