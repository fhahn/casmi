#include <cassert>
#include <sstream>

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

  std::string arguments_to_string(const Function *func, const uint64_t args[], 
                                  uint16_t sym_args, bool strip=false) {
    std::stringstream ss;
    ss << ',';

    for (uint32_t i = 0; i < func->arguments_.size(); i++) {
      if ((sym_args & (1 << i)) != 0) {
        ss << "sym" << (INT_T) args[i];
      } else {
        switch (func->arguments_[i]->t) {
          case TypeType::INT:
            ss << (INT_T) args[i];
            break;
          default: assert(0);
        }
      }
      ss << ',';
    }
    // Strip leading and trailing comma if requested
    if (strip) {
      return ss.str().substr(1, ss.str().size()-2);
    } else {
      return ss.str();
    }
  }


  std::string location_to_string(const Function *func, const uint64_t args[],
                                 uint16_t sym_args, const Value& val, uint32_t time) {
    std::stringstream ss;
    ss << "st" << func->name << "(" << time
       << arguments_to_string(func, args, sym_args) << val.to_str()
       << ")";
    return ss.str();
  }


  void dump_create(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const Value& v) {
    std::stringstream ss;
    ss << "tff(symbolNext, type, sym" << v.value.ival<< ": $int)."
       << std::endl;
    ss << "fof(id"<<next_fof_id() << ",hypothesis,"
       << location_to_string(func, args, sym_args, v, get_timestamp())
       << ").%CREATE: " << func->name
       << '(' << arguments_to_string(func, args, sym_args, true) << ')' << std::endl;
    trace.push_back(ss.str());
  }

  void dump_update(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const Value& v) {
    std::stringstream ss;
    ss << "fof(id" << next_fof_id() << ",hypothesis,"
       << location_to_string(func, args, sym_args, v, get_timestamp())
       << ").%UPDATE: " << func->name
       << '(' << arguments_to_string(func, args, sym_args, true) << ')' << std::endl;
    trace.push_back(ss.str());
  }

  void dump_if(std::vector<std::string>& trace, const std::string &filename,
      size_t lineno, const Value &sym) {
    std::stringstream ss;
    ss << "fof('id" << filename <<  ":" << lineno << "',hypothesis,"
       << sym.to_str() << ")." << std::endl;
    trace.push_back(ss.str());
  }

  void dump_final(std::vector<std::string>& trace, const std::vector<std::pair<const Function*,
      std::unordered_map<ArgumentsKey, Value> >>& functions) {
    std::stringstream ss;
    uint32_t i = 0;
    for (auto& pair : functions) {
      if (!pair.first->is_symbolic) {
        continue;
      }
      for (auto& value_pair : pair.second) {
        ss << "fof(final" << i << ",hypothesis,"
           << location_to_string(pair.first, value_pair.first.p, value_pair.first.sym_args, value_pair.second, 0)
           << ").%FINAL: " << pair.first->name << '(' 
           << arguments_to_string(pair.first, value_pair.first.p, value_pair.first.sym_args, true)
           << ')' << std::endl;
      }
      i += 1;
    }
    trace.push_back(ss.str());
  }
}
