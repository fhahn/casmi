#ifndef CASMI_LIBINTERPRETER_SYMBOLIC_H
#define CASMI_LIBINTERPRETER_SYMBOLIC_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "libsyntax/symbols.h"
#include "libinterpreter/value.h"
#include "libinterpreter/execution_context.h"

namespace symbolic {
  uint32_t next_symbol_id();
  uint32_t next_fof_id();
  void advance_timestamp();
  uint32_t get_timestamp();

  void dump_create(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const Value& v);
  void dump_update(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const Value& v);
  void dump_if(std::vector<std::string>& trace, const std::string &filename,
      size_t lineno, const Value& sym);
  void dump_final(std::vector<std::string>& trace,
      const std::vector<std::pair<const Function*, 
      std::unordered_map<ArgumentsKey, Value> >>& functions);

  enum class check_status_t {
    NOT_FOUND,
    TRUE,
    FALSE
  };

  check_status_t check_condition(std::vector<symbolic_condition*> known_condition,
      const symbolic_condition *check);
};

#endif
