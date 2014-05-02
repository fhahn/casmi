#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include <vector>
#include <unordered_map>

#include "libsyntax/symbols.h"
#include "libsyntax/driver.h"

#include "libinterpreter/value.h"

#include "librt/rt.h"

#define UPDATESET_SIZE 65535


struct ArgumentsKey {
  uint64_t* p;
  size_t size;

  inline bool operator==(const ArgumentsKey other) const {
    if(other.size == size) {
      for(size_t i = 0; i < size; i++) {
        if (other.p[i] != p[i]) {
          DEBUG(other.p[i] << " != "<<p[i] << " at "<<i);
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  }
};


namespace std {
  template <> struct hash<ArgumentsKey> {

    size_t operator()(const ArgumentsKey &key) const {
        size_t h = 0;
        hash<uint64_t> hasher;
        for(size_t i = 0; i < key.size; i++) {
          h ^= hasher(key.p[i]);
        }
        return h;
    }
  };
}


class ExecutionContext {
  private:
    pp_mem updateset_data_;
    std::map<const std::string, bool> debuginfo_filters;

  public:
    std::vector<std::pair<Function*, std::unordered_map<ArgumentsKey, Value> >> functions;
    const SymbolTable<Function*> symbol_table;
    casm_updateset updateset;
    pp_mem pp_stack;
    std::vector<List*> temp_lists;

    ExecutionContext(const SymbolTable<Function*>& st, RuleNode *init);

    void apply_updates();
    void merge_par();
    void merge_seq(Driver& driver);

    void set_function(Function *sym, uint64_t args[], Value& val);
    Value& get_function_value(Function *sym, uint64_t args[]);

    bool set_debuginfo_filter(const std::string& filters);
    bool filter_enabled(const std::string& filter);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
