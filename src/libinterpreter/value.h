#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H


#include "libsyntax/ast.h"
#include "libsyntax/types.h"

class Value {
  public:
    const Type type;
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

    void add(Value& other);
    void sub(Value& other);
    void mul(Value& other);
    void div(Value& other);
    void mod(Value& other);

    uint64_t to_uint64_t() const;
};

#endif
