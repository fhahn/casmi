#ifndef CASMI_LIBPARSE_TYPES_H
#define CASMI_LIBPARSE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>

#define INT_T int64_t
#define FLOAT_T double
enum class TypeType {
  STRING,
  RULEREF,
  INT,
  FLOAT,
  BOOLEAN,
  SELF,
  UNKNOWN,
  INVALID,
  NO_TYPE,
  UNDEF
};


class Type {
  public:
    TypeType t;

    Type();
    Type(TypeType t);
    Type(const std::string& type_name);


    inline bool operator==(const Type& other) const {
      return t == other.t;
    }

    inline bool operator!=(const Type& other) const {
      return !(t == other.t);
    }

    const std::string to_str() const;
};




class FunctionInfo {
  private:
    std::vector<Type> *arguments_;
    Type return_type_;

  public:
    FunctionInfo(std::vector<Type> *args, Type return_type);
};

#endif
