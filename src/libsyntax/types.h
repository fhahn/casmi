#ifndef CASMI_LIBPARSE_TYPES_H
#define CASMI_LIBPARSE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>

#define INT_T int32_t
#define FLOAT_T double

enum class Type { RULE, INT, FLOAT, BOOL, UNKNOWN, INVALID, NO_TYPE, UNDEF };


Type str_to_type(const std::string& type_name);
const std::string type_to_str(Type t);


class FunctionInfo {
  private:
    std::vector<Type> *arguments_;
    Type return_type_;

  public:
    FunctionInfo(std::vector<Type> *args, Type return_type);
};

#endif
