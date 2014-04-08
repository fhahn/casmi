#include <iostream>

#include "libsyntax/symbols.h"


// -------------------------------------------------------------------------
// Implementation of Symbol
// -------------------------------------------------------------------------

int Symbol::counter = 0;

Symbol::Symbol(const std::string name, std::vector<Type> *args,
              Type return_type) : name_(name), arguments_(args),
                                  return_type_(return_type), id(counter) {
  counter += 1;
}

Symbol::Symbol(const std::string name) :
        name_(name), arguments_(nullptr), return_type_(Type::UNKNOWN), id(counter) {
  counter += 1;
}


Symbol::~Symbol() {
  delete arguments_;
}

const std::string& Symbol::name() const {
  return name_;
}

const std::string Symbol::to_str() const {
  std::string res = name_;

  if (arguments_ == nullptr) {
    res += ": ()";
  } else {
    res = ": (";
    for (Type t : *arguments_) {
      res += type_to_str(t) + ", ";
    }
    res += ")";
  }
  res += "-> "+type_to_str(return_type_);
  return res;
}

bool Symbol::equals(Symbol *other) const {
  if (name_ != other->name_) {
    return false;
  }

  if (arguments_ != nullptr && other->arguments_ != nullptr) {
    if (arguments_->size() != other->arguments_->size()) {
      return false;
    }
    for (size_t i=0; i < arguments_->size(); i++) {
      if ((*arguments_)[i] != (*other->arguments_)[i]) {
        return false;
      }
    }
  }
  return return_type_ == other->return_type_;
}

SymbolUsage::SymbolUsage(yy::location& loc, const std::string name) :
        name_(name), arguments_(nullptr), location(loc) {}

SymbolUsage::SymbolUsage(yy::location& loc, const std::string name,
                         std::vector<Expression*> *args) :
        name_(name), arguments_(args), location(loc) {}


SymbolUsage::~SymbolUsage() {
  if (arguments_ != nullptr) {
    for (auto a : *arguments_) {
      delete a;
    }
    delete arguments_;
  }
}

bool SymbolUsage::equals(SymbolUsage *other) const {
  if (arguments_ != nullptr && other->arguments_ != nullptr) {
    if (arguments_->size() != other->arguments_->size()) {
      return false;
    }
    for (size_t i=0; i < arguments_->size(); i++) {
      if ((*arguments_)[i] != (*other->arguments_)[i]) {
        return false;
      }
    }
  }
  return arguments_ == other->arguments_ && name_ == other->name_;
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
    DEBUG("Add symbol "+sym->name());
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

Type SymbolTable::get(const SymbolUsage *usage) const {
  Symbol *sym = get(usage->name_);
  if (sym == nullptr) {
    DEBUG("did not find symbol with name "+usage->name_);
    return Type::INVALID;
  } else {
    DEBUG("found symbol with name "+usage->name_);
    return sym->return_type_;
  }
}

size_t SymbolTable::size() const {
  return table_.size();
}
