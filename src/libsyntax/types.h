#ifndef CASMI_LIBPARSE_TYPES_H
#define CASMI_LIBPARSE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>

#define INT_T int64_t
#define FLOAT_T double

class CompoundType;

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
  UNDEF,
  LIST,
  TUPLE,
};

class Type;

class Type {
  private:
    bool eq(const Type& other) const;

  public:
    TypeType t;
    Type *internal_type;
    Type *unify_with_left;
    Type *unify_with_right;

    std::vector<Type*> constraints;

    Type();
    Type(TypeType t);
    Type(const Type& other);
    Type(const std::string &type_name, std::vector<Type*>& internal_types);
    Type(TypeType t, std::vector<Type*>& internal_types);
    Type(TypeType t, Type *int_typ);
    Type(const std::string& type_name);
    Type(Type *other);


    bool operator==(const Type& other) const;
    bool operator==(const TypeType other) const;

    bool operator!=(const Type& other) const;

    virtual const std::string to_str() const;
    virtual std::string unify_links_to_str() const;
    virtual std::string unify_links_to_str_left() const;
    virtual std::string unify_links_to_str_right() const;

    // unify with temporary type
    bool unify(Type other);

    // unify types of ast nodes
    bool unify(Type *other);

    bool unify_right(Type *other);

    bool unify_left(Type *other);

    bool unify_nofollow(Type *other);


    bool is_complete();
};


class FunctionInfo {
  private:
    std::vector<Type> *arguments_;
    Type return_type_;

  public:
    FunctionInfo(std::vector<Type> *args, Type return_type);
};

#endif
