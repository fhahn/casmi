#include <string>
#include <map>

#include "libsyntax/types.h"

Type str_to_type(const std::string& type_name) {
  if (type_name == "Int") return Type::INT;
  if (type_name == "Float") return Type::FLOAT;
  if (type_name == "Undef") return Type::UNDEF;
  if (type_name == "Boolean") return Type::BOOLEAN;
  if (type_name == "RuleRef") return Type::RULEREF;
  return Type::INVALID;
}


const std::string type_to_str(Type t) {
  if (t == Type::INT) return "Int";
  if (t == Type::FLOAT) return "Float";
  if (t == Type::UNDEF) return "Undef";
  if (t == Type::BOOLEAN) return "Boolean";
  if (t == Type::RULEREF) return "RuleRef";
  return "Invalid";
}

FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
