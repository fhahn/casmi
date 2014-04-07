#include <string>
#include <map>

#include "libsyntax/types.h"

Type str_to_type(const std::string& type_name) {
  if (type_name == "Int") return Type::INT;
  if (type_name == "Float") return Type::FLOAT;
  if (type_name == "Undef") return Type::UNDEF;
  if (type_name == "Bool") return Type::BOOL;
  return Type::INVALID;
}


const std::string type_to_str(Type t) {
  if (t == Type::INT) return "Int";
  if (t == Type::FLOAT) return "Float";
  if (t == Type::UNDEF) return "Undef";
  if (t == Type::BOOL) return "Bool";
  return "Invalid";
}

FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
