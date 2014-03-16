#ifndef CASMI_LIBPARSE_SYMBOLS_H
#define CASMI_LIBPARSE_SYMBOLS_H

#include <string>
#include <map>
#include <vector>

#include "libparse/ast.h"
#include "libparse/types.h"

class Expression;

// Only used for functions at the moment
class Symbol {
  private:
    const std::string name_;
    std::vector<Type> *arguments_;

  public:
    Type return_type_;

    Symbol(const std::string name, std::vector<Type> *args, Type return_type);
    Symbol(const std::string name);
    ~Symbol();

    const std::string& name() const;
    bool equals(Symbol *other) const;
};

class SymbolUsage {
  private:
    std::vector<Expression*> *arguments_;

  public:
    const std::string name_;

    SymbolUsage(const std::string name);
    SymbolUsage(const std::string name, std::vector<Expression*> *args);
    ~SymbolUsage();
    bool equals(SymbolUsage *other) const;
};

class SymbolTable {
  private:
    std::map<std::string, Symbol*> table_;
    SymbolTable *outer_scope_;

  public:
    SymbolTable();
    SymbolTable(SymbolTable *outer);
    ~SymbolTable();

    size_t size() const;
    bool add(Symbol *s);
    Symbol *get(const std::string& name) const;
    Type get(const SymbolUsage *sym) const;
};

#endif
