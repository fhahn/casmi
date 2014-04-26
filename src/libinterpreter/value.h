#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H


#include "libsyntax/types.h"

#include "librt/rt.h"

class RuleNode;


class List {
  public:
    enum class ListType {
      TEMP,
      PERM,
    };

    ListType list_type;

    List(ListType t);

    virtual const std::string to_str() const { return ""; }
};


class Value {
  public:
    Type type;
    union {
      INT_T ival;
      FLOAT_T fval;
      bool bval;
      RuleNode *rule;
      std::string *string;
      List *list;
    } value;

    Value();
    Value(INT_T ival);
    Value(FLOAT_T fval);
    Value(bool bval);
    Value(RuleNode *rule);
    Value(std::string *string);
    Value(Type t, List *list);

    Value(Value& other);
    Value(const Value& other);
    //Value(Value&& other);
    Value(Type type, casm_update* update);

    void add(const Value& other);
    void sub(const Value& other);
    void mul(const Value& other);
    void div(const Value& other);
    void mod(const Value& other);
    void rat_div(const Value& other);

    void eq(const Value& other);

    void lesser(const Value& other);
    void greater(const Value& other);
    void lessereq(const Value& other);
    void greatereq(const Value& other);

    uint64_t to_uint64_t() const;

    bool is_undef() const;

    std::string to_str() const;
};


bool value_eq(const Value& v1, const Value& v2);


class TempList : public List {
  public:
    List* left;
    List* right;
    std::vector<Value> changes;

    TempList();

    const std::string to_str() const;
};

class PermList : public List {
  public:
    std::vector<Value> values;

    PermList();

    const std::string to_str() const;
};


#endif
