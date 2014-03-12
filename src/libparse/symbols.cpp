#include "libparse/symbols.h"

// -------------------------------------------------------------------------
// Implementation of Symbol
// -------------------------------------------------------------------------
Symbol::Symbol(const std::string name) : name_(name), type(Type::UNKNOWN) {}


const std::string& Symbol::name() const {
  return name_;
}
// -------------------------------------------------------------------------
// Implementation of SymbolTable
// -------------------------------------------------------------------------
SymbolTable::SymbolTable() : outer_scope_(nullptr) {}

SymbolTable::SymbolTable(SymbolTable *outer) : outer_scope_(outer) {}

SymbolTable::~SymbolTable() {
  // cleanup symbol table
  for (auto entry : table_) {
    delete entry.second;
  }
}

Symbol *SymbolTable::add_symbol(const std::string& s) {
  try {
    return table_.at(s);
  } catch (const std::out_of_range& e) {
    Symbol *sym = new Symbol(s);
    table_[s] = sym;
    return sym;
  }
}

Symbol *SymbolTable::get(const std::string& name) const {
  try {
    return table_.at(name);
  } catch (const std::out_of_range& e) {
    return nullptr;
  }
}

size_t SymbolTable::size() const {
  return table_.size();
}
