#include <iostream>

#include "libsyntax/symbols.h"


// -------------------------------------------------------------------------
// Implementation of Function
// -------------------------------------------------------------------------

int Function::counter = 0;

Function::Function(const std::string name, std::vector<Type> *args,
              Type return_type, std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *init) :
                name_(name), arguments_(args), intitializers_(init),
                return_type_(return_type), id(counter) {
  counter += 1;
}

Function::Function(const std::string name) :
        name_(name), arguments_(nullptr), intitializers_(nullptr),
        return_type_(Type::UNKNOWN), id(counter) {
  counter += 1;
}

Function::~Function() {
  delete arguments_;
  if (intitializers_ != nullptr) {
    for (std::pair<ExpressionBase*, ExpressionBase*> e : *intitializers_) {
      delete e.first;
      delete e.second;
    }
    delete intitializers_;
  }
}

const std::string& Function::name() const {
  return name_;
}

const std::string Function::to_str() const {
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

bool Function::equals(Function *other) const {
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
// Implementation of FunctionTable
// -------------------------------------------------------------------------
FunctionTable::FunctionTable() {}

//FunctionTable::FunctionTable(FunctionTable *outer) : outer_scope_(outer) {}

FunctionTable::~FunctionTable() {
  // cleanup symbol table
  for (auto entry : table_) {
    delete entry.second;
  }
}

bool FunctionTable::add(Function *sym) {
  try {
    table_.at(sym->name());
    return false;
  } catch (const std::out_of_range& e) {
    DEBUG("Add symbol "+sym->name());
    table_[sym->name()] = sym;
    return true;
  }
}

Function *FunctionTable::get(const std::string& name) const {
  try {
    return table_.at(name);
  } catch (const std::out_of_range& e) {
    return nullptr;
  }
}

size_t FunctionTable::size() const {
  return table_.size();
}
