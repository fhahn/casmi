#ifndef CASMI_LIBPARSE_SYMBOLS_H
#define CASMI_LIBPARSE_SYMBOLS_H

#include <string>
#include <map>
#include <vector>

#include "libsyntax/ast.h"
#include "libsyntax/types.h"
#include "libsyntax/location.hh" // reuse bison's location class

class ExpressionBase;
class Expression;
class FunctionAtom;

class Function {
  private:
    static uint64_t counter;
    const std::string name_;

  public:
    std::vector<Type> *arguments_;
    std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *intitializers_;
    Type return_type_;
    const uint64_t id;

    Function(const std::string name, std::vector<Type> *args, Type return_type,
           std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *init);
    Function(const std::string name);
    ~Function();

    const std::string& name() const;
    bool equals(Function *other) const;
    const std::string to_str() const;
    inline size_t argument_count() const {
      if (arguments_ == nullptr) return 0;
      return arguments_->size();
    }
};

class Binding {
  private:
    static uint64_t counter;

  public:
    const uint64_t id;
    const std::string name;
    Type type;

    Binding(const std::string& name);
    Binding(const std::string& name, Type t);
};

template<typename T>
class SymbolTable {
  public:
    std::map<std::string, T> table_;

    SymbolTable() {}

    ~SymbolTable() {

      // cleanup symbol table
      /* TODO: check if element wise cleanup is needed
      for (auto entry : table_) {
        delete entry.second;
      }
      */
    }

    size_t size() const {
      return table_.size();
    }

    bool add(T sym) {
      try {
        table_.at(sym->name());
        return false;
      } catch (const std::out_of_range& e) {
        DEBUG("Add symbol "+sym->name());
        table_[sym->name()] = sym;
        return true;
      }
    }

    T get(const std::string& name) const {
      try {
        return table_.at(name);
      } catch (const std::out_of_range& e) {
        return nullptr;
      }
    }

    Type get(const FunctionAtom *func) const;
};

#endif
