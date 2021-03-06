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
  LIST,
  TUPLE,
  TUPLE_OR_LIST,
  ENUM,
  RATIONAL,
  UNDEF,    // only possible during execution in value_t.type
  SYMBOL,   // only possible during execution in value_t.type
};

class Type;


class Type {
  private:
    bool eq(const Type& other) const;
    bool unify_list(Type *other);
    bool unify_tuple(Type *other);
    bool unify_tuple_or_list(Type *other);
    bool unify_enum(Type *other);

  public:
    TypeType t;
    Type *unify_with_left;
    Type *unify_with_right;

    std::vector<Type*> constraints;
    std::vector<Type*> subtypes;

    std::string enum_name;

    INT_T subrange_start = 0;
    INT_T subrange_end = -1;

    Type();
    Type(TypeType t);
    Type(const Type& other);
    Type(const std::string &type_name, std::vector<Type*>& internal_types);
    Type(TypeType t, std::vector<Type*>& internal_types);
    Type(TypeType t, Type *int_typ);
    Type(const std::string& type_name);
    Type(TypeType enum_type, const std::string& type_name);
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


    const Type* get_most_general_type() const;
    bool is_complete() const;
    bool is_unknown() const;
};


#endif
