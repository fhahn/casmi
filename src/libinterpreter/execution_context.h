#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include <vector>
#include <unordered_map>

#include "libsyntax/symbols.h"
#include "libsyntax/driver.h"

#include "libinterpreter/value.h"

#include "libcasmrt/rt.h"

#define UPDATESET_SIZE 65535


struct ArgumentsKey {
  uint64_t* p;
  bool dynamic;


  ArgumentsKey(uint64_t *args, uint16_t size, bool dyn);
  ArgumentsKey(const ArgumentsKey& other);
  ArgumentsKey(ArgumentsKey&& other) noexcept;
  ~ArgumentsKey();
};



namespace std {

  template <> struct hash<ArgumentsKey> {
    std::vector<Type*> types;

    size_t operator()(const ArgumentsKey &key) const {
      size_t h = 0;
      for(uint16_t i = 0; i < types.size(); i++) {
        h ^= hash_uint64_value(types[i], key.p[i]);
      }
      return h;
    }
  };

  template <> struct equal_to<ArgumentsKey> {
    std::vector<Type*> types;

    bool operator()(const ArgumentsKey &lhs, const ArgumentsKey &rhs) const {
      for(uint16_t i = 0; i < types.size(); i++) {
        if (!eq_uint64_value(types[i], lhs.p[i], rhs.p[i])) {
          return false;
        }
      }
      return true;
    }
  };

}


class ExecutionContext {
  private:
    pp_mem updateset_data_;
    std::map<const std::string, bool> debuginfo_filters;

  public:
    std::vector<std::pair<const Function*, std::unordered_map<ArgumentsKey, Value> >> functions;
    const SymbolTable symbol_table;
    casm_updateset updateset;
    pp_mem pp_stack;
    std::vector<List*> temp_lists;
    static pp_mem value_stack;
    const bool symbolic;

    ExecutionContext(const SymbolTable& st, RuleNode *init, const bool symbolic);
    ExecutionContext(const ExecutionContext& other);

    void apply_updates();
    void merge_par();
    void merge_seq(Driver& driver);

    void set_function(Function *sym, uint64_t args[], Value& val);
    Value& get_function_value(Function *sym, uint64_t args[]);

    bool set_debuginfo_filter(const std::string& filters);
    bool filter_enabled(const std::string& filter);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
