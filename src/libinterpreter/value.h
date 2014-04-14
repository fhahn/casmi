#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H


#include "libsyntax/ast.h"
#include "libsyntax/types.h"

class Value {
  public:
    Type type;
    union {
      INT_T ival;
      FLOAT_T fval;
      bool bval;
      RuleNode *rule;
    } value;

    Value();
    Value(INT_T ival);
    Value(FLOAT_T fval);
    Value(bool bval);
    Value(RuleNode *rule);
    Value(Value&& other);

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
};

#endif
