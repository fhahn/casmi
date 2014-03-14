#include "libparse/symbols.h"
#include <iostream>


// -------------------------------------------------------------------------
// Implementation of Symbol
// -------------------------------------------------------------------------

Symbol::Symbol(const std::string name, std::vector<Type> *types) : name_(name), types_(types) {}

Symbol::Symbol(const std::string name) : name_(name) {
  types_ = new std::vector<Type>;
  types_->push_back(Type::UNKNOWN);
}

Symbol::~Symbol() {
  delete types_;
}

const std::string& Symbol::name() const {
  return name_;
}

bool Symbol::equals(Symbol *other) const {
  if (name_ != other->name_ || types_->size() != other->types_->size()) {
    return false;
  }
  for (int i=0; i < types_->size(); i++) {
    if ((*types_)[i] != (*other->types_)[i]) {
      std::cout << type_to_str((*types_)[i]) << "\n";
      return false;
    }
  }
  return true;
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
