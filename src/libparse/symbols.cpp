#include "libparse/symbols.h"


// -------------------------------------------------------------------------
// Implementation of Symbol
// -------------------------------------------------------------------------
Symbol::Symbol(const std::string name) : name_(name), type(Type::UNKNOWN) {}


const std::string& Symbol::name() const {
  return name_;
}

bool Symbol::equals(Symbol *other) const {
  return type == other->type && name_ == other->name_;
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

bool SymbolTable::add(Symbol *sym) {
  try {
    table_.at(sym->name());
    return false;
  } catch (const std::out_of_range& e) {
    table_[sym->name()] = sym;
    return true;
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
