#ifndef CASMI_LIBPARSE_SYMBOLS_H
#define CASMI_LIBPARSE_SYMBOLS_H

#include <string>
#include <map>
#include <vector>

#include "libsyntax/ast.h"
#include "libsyntax/types.h"
#include "libsyntax/location.hh" // reuse bison's location class

class Expression;

// Only used for functions at the moment
class Symbol {
  private:
    static int counter;
    const std::string name_;
    std::vector<Type> *arguments_;

  public:
    Type return_type_;
    const int id;

    Symbol(const std::string name, std::vector<Type> *args, Type return_type);
    Symbol(const std::string name);
    ~Symbol();

    const std::string& name() const;
    bool equals(Symbol *other) const;
    const std::string to_str() const;
};

class SymbolUsage {
  private:
    std::vector<Expression*> *arguments_;

  public:
    const std::string name_;
    yy::location location;
    Symbol *symbol;

    SymbolUsage(yy::location& loc, const std::string name);
    SymbolUsage(yy::location& loc, const std::string name, std::vector<Expression*> *args);
    ~SymbolUsage();
    bool equals(SymbolUsage *other) const;
};

class SymbolTable {
  private:
    SymbolTable *outer_scope_;

  public:
    std::map<std::string, Symbol*> table_;

    SymbolTable();
    SymbolTable(SymbolTable *outer);
    ~SymbolTable();

    size_t size() const;
    bool add(Symbol *s);
    Symbol *get(const std::string& name) const;
    Type get(const SymbolUsage *sym) const;
};

#endif
