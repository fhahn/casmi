#include <iostream>

#include "libsyntax/symbols.h"

static std::map<const std::string, bool> builtin_names = {
  {"pow", true},
  {"hex", true},
  {"nth", true},
  {"cons", true},
  {"len", true},
  {"tail", true},
  {"peek", true},
};

bool is_builtin_name(const std::string& name) {
  if(builtin_names.count(name) != 0) {
    return true;
  }
  return false;
}

// -------------------------------------------------------------------------
// Implementation of Function
// -------------------------------------------------------------------------

uint64_t Function::counter = 0;

Function::Function(const std::string name, std::vector<Type*>& args,
              Type* return_type, std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *init) :
                name_(name), arguments_(std::move(args)), intitializers_(init),
                return_type_(return_type), id(counter), symbol_type(SType::FUNCTION) {
  counter += 1;
}

Function::Function(const std::string name, std::vector<Type*>& args,
                   ExpressionBase *expr, Type* return_type) :
                name_(name), arguments_(std::move(args)), derived(expr),
                return_type_(return_type), id(counter), symbol_type(SType::DERIVED) {
  counter += 1;
}

Function::Function(const std::string name,
                   ExpressionBase *expr, Type* return_type) :
                name_(name), arguments_(), derived(expr),
                return_type_(return_type), id(counter), symbol_type(SType::DERIVED) {
  counter += 1;
}


Function::~Function() {
  arguments_.clear();
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

  res = ": (";
  for (Type* t : arguments_) {
    res += t->to_str() + ", ";
  }
  res += ")";
  res += "-> "+return_type_->to_str();
  return res;
}

bool Function::equals(Function *other) const {
  if (name_ != other->name_) {
    return false;
  }

  if (arguments_.size() != other->arguments_.size()) {
    return false;
  }
  for (size_t i=0; i < arguments_.size(); i++) {
    if (arguments_[i] != other->arguments_[i]) {
      return false;
    }
  }
  return return_type_ == other->return_type_;
}

bool Function::is_builtin() {
  if(builtin_names.count(name_) != 0) {
    symbol_type = SType::BUILTIN;
    return true;
  }
  return false;
}

uint64_t Binding::counter = 0;

Binding::Binding(const std::string& name) : Binding(name, Type(TypeType::UNKNOWN)) {}

Binding::Binding(const std::string& name, Type t) : id(counter), name(name),
    type(t)
{
  counter += 1;
}
