#include <iostream>

#include "libsyntax/symbols.h"


// -------------------------------------------------------------------------
// Implementation of Symbol
// -------------------------------------------------------------------------

int Symbol::counter = 0;

Symbol::Symbol(const std::string name, std::vector<Type> *args,
              Type return_type, std::vector<AtomNode*> *init) :
                name_(name), arguments_(args), intitializers_(init),
                return_type_(return_type), id(counter) {
  counter += 1;
}

Symbol::Symbol(const std::string name) :
        name_(name), arguments_(nullptr), intitializers_(nullptr),
        return_type_(Type::UNKNOWN), id(counter) {
  counter += 1;
}

Symbol::~Symbol() {
  delete arguments_;
  if (intitializers_ != nullptr) {
    for (AtomNode* e : *intitializers_) {
      delete e;
    }
    delete intitializers_;
  }
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

// -------------------------------------------------------------------------
// Implementation of SymbolTable
// -------------------------------------------------------------------------
SymbolTable::SymbolTable() {}

//SymbolTable::SymbolTable(SymbolTable *outer) : outer_scope_(outer) {}

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

size_t SymbolTable::size() const {
  return table_.size();
}
