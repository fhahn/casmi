#ifndef CASMI_LIBINTERPRETER_EXEC_CONTEXT
#define CASMI_LIBINTERPRETER_EXEC_CONTEXT

#include <vector>
#include <unordered_map>

#include "libsyntax/symbols.h"

#include "libinterpreter/value.h"

#include "librt/pp_hashmap.h"
#include "librt/pp_casm_updateset_linkedhash.h"

#define UPDATESET_SIZE 65535


CASM_UPDATE_TYPE(4);

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

    size_t operator()(const ArgumentsKey &dr) const
    {
        size_t h = 0;
        hash<uint64_t> hasher;
        for (uint64_t* p = dr.p; p != dr.p + dr.size; ++p)
            h ^= hasher(*p);
        return h;
    }
  };
}


class ExecutionContext {
  private:
    pp_mem updateset_data_;
    std::vector<std::unordered_map<ArgumentsKey, casm_update*> > functions;
    std::vector<Symbol*> syms_to_apply;

  public:
    casm_updateset updateset;
    pp_mem pp_stack;
    uint64_t pseudostate;

    void apply_updates();
    void set_function(Symbol *sym, casm_update *update);
    ExecutionContext(SymbolTable *st);
};

#endif //CASMI_LIBINTERPRETER_EXEC_CONTEXT
