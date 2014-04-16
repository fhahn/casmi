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
    static int counter;
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

class FunctionTable {
  private:
    //FunctionTable *outer_scope_;

  public:
    std::map<std::string, Function*> table_;

    FunctionTable();
    //FunctionTable(FunctionTable *outer);
    ~FunctionTable();

    size_t size() const;
    bool add(Function *s);
    Function *get(const std::string& name) const;
    Type get(const FunctionAtom *func) const;
};

#endif
