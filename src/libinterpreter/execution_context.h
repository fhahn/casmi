#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include <vector>
#include <unordered_map>

#include "libsyntax/symbols.h"

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
    std::vector<Symbol*> syms_to_apply;

  public:
    std::vector<std::pair<Symbol*, std::unordered_map<ArgumentsKey, Value> >> functions;
    SymbolTable *symbol_table;
    casm_updateset updateset;
    pp_mem pp_stack;
    uint64_t pseudostate;

    void apply_updates();
    void set_function(Symbol *sym, uint64_t args[], Value& val);
    Value& get_function_value(Symbol *sym, uint64_t args[]);
    ExecutionContext(SymbolTable *st, RuleNode *init);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
