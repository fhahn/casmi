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


Symbol::Symbol(const std::string& name, SymbolType type) : name(std::move(name)), type(type) {}

// -------------------------------------------------------------------------
// Implementation of Function
// -------------------------------------------------------------------------

uint64_t Function::counter = 0;

Function::Function(const std::string name, std::vector<Type*>& args,
              Type* return_type, std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *init) :
                Symbol(name, SymbolType::FUNCTION), arguments_(std::move(args)), intitializers_(init),
                return_type_(return_type), id(counter), is_static(false), is_symbolic(false) {
  counter += 1;
}

Function::Function(bool is_static, bool is_symbolic, const std::string name,
             std::vector<Type*>& args, Type* return_type,
             std::vector<std::pair<ExpressionBase*, ExpressionBase*>> *init) :
                Symbol(name, SymbolType::FUNCTION), arguments_(std::move(args)), intitializers_(init),
                return_type_(return_type), id(counter),
                is_static(is_static), is_symbolic(is_symbolic) {

  counter += 1;
}

Function::Function(const std::string name, std::vector<Type*>& args,
                   ExpressionBase *expr, Type* return_type) :
                Symbol(name, SymbolType::DERIVED), arguments_(std::move(args)), derived(expr),
                return_type_(return_type), id(counter),
                is_static(false), is_symbolic(false) {
  counter += 1;
}

Function::Function(const std::string name,
                   ExpressionBase *expr, Type* return_type) :
                Symbol(name, SymbolType::DERIVED), arguments_(), derived(expr),
                return_type_(return_type), id(counter), is_static(false), is_symbolic(false) {
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

const std::string Function::to_str() const {
  std::string res = name;

  res = ": (";
  for (Type* t : arguments_) {
    res += t->to_str() + ", ";
  }
  res += ")";
  res += "-> "+return_type_->to_str();
  return res;
}

bool Function::equals(Function *other) const {
  if (name != other->name) {
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
  if(builtin_names.count(name) != 0) {
    type = SymbolType::BUILTIN;
    return true;
  }
  return false;
}

Enum::Enum(const std::string& name) : Symbol(name, Symbol::SymbolType::ENUM), mapping() {}

bool Enum::add_enum_element(const std::string& name) {
  if (mapping.count(name) == 0) {
    mapping[name] = mapping.size();
    return true;
  } else {
    return false;
  }
}

uint64_t Binding::counter = 0;

Binding::Binding(const std::string& name) : Binding(name, Type(TypeType::UNKNOWN)) {}

Binding::Binding(const std::string& name, Type t) : id(counter), name(name),
    type(t)
{
  counter += 1;
}
