#include <string>
#include <map>

#include "libparse/types.h"

Type str_to_type(const std::string& type_name) {
  if (type_name == "Int") return Type::INT;
  if (type_name == "Float") return Type::FLOAT;
  return Type::INVALID;
}


const std::string type_to_str(Type t) {
  if (t == Type::INT) return "Int";
  if (t == Type::FLOAT) return "Float";
  return "Invalid";
}


FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
