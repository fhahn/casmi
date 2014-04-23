#include <string>
#include <map>

#include "macros.h"

#include "libsyntax/types.h"

Type::Type(const std::string& type_name, std::vector<Type>& internal_types) {
  if (type_name == "List") {
    t = TypeType::LIST;
  } else {
    t = TypeType::INVALID;
  }

  if (internal_types.size() == 0) {
    internal_type = new Type(TypeType::UNKNOWN);
  } else if (internal_types.size() == 1) {
    internal_type = new Type(internal_types[0]);
  } else {
    t = TypeType::INVALID;
  }
}

Type::Type(TypeType typ, std::vector<Type>& internal_types) : t(typ) {
  if (t != TypeType::LIST && t != TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
  if (internal_types.size() == 0) {
    internal_type = new Type(TypeType::UNKNOWN);
  } else if (internal_types.size() == 1) {
    internal_type = new Type(internal_types[0]);
  } else {
    t = TypeType::INVALID;
  }
}

Type::Type(TypeType typ, Type& int_typ) : t(typ) {
  if (t != TypeType::LIST && t != TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
  internal_type = new Type(int_typ);
}

Type::Type() : t(TypeType::INVALID), internal_type(nullptr) {}

Type::Type(TypeType t) : t(t), internal_type(nullptr) {
  if (t == TypeType::LIST || t == TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
}

Type::Type(const Type& other) : t(other.t), internal_type(other.internal_type) {}

Type::Type(const std::string& type_name) {
  if (type_name == "Int") { t = TypeType::INT; }
  else if (type_name == "Float") { t = TypeType::FLOAT; }
  else if (type_name == "Undef") { t = TypeType::UNDEF; }
  else if (type_name == "Boolean") { t = TypeType::BOOLEAN; }
  else if (type_name == "RuleRef") { t = TypeType::RULEREF; }
  else if (type_name == "String") { t = TypeType::STRING; }
  else { t = TypeType::INVALID; }
}

bool Type::eq(const Type& other) const {
  if (t != other.t) {
    return false;
  }

  if (t == TypeType::LIST) {
    return *internal_type == *other.internal_type;
  }

  return true;
}

bool Type::operator==(const Type& other) const {
  return eq(other);
}

bool Type::operator!=(const Type& other) const {
  return !eq(other);
}

const std::string Type::to_str() const {
  switch (t) {
    case TypeType::INT: return "Int";
    case TypeType::FLOAT: return "Float";
    case TypeType::UNDEF: return "Undef";
    case TypeType::BOOLEAN: return "Boolean";
    case TypeType::RULEREF: return "RuleRef";
    case TypeType::STRING: return "String";
    case TypeType::LIST: return "List("+internal_type->to_str()+")";
    default: return "Invalid";
  }
}

FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
