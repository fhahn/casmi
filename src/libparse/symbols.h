#ifndef CASMI_LIBPARSE_SYMBOLS_H
#define CASMI_LIBPARSE_SYMBOLS_H

#include <string>
#include <map>

#include "libparse/types.h"


class Symbol {
  private:
    const std::string name_;
  public:
    Type type;
    Symbol(const std::string name);

    const std::string& name() const;
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
    Symbol *add_symbol(const std::string& s);
    Symbol *get(const std::string& name) const;
};

#endif
