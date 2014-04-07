#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H

#include "libsyntax/types.h"

class Value {
  public:
    const Type type;
    union {
      INT_T ival;
      FLOAT_T fval;
      bool bval;
    } value;

    Value();
    Value(INT_T ival);
    Value(FLOAT_T fval);
    Value(bool bval);
    Value(Value&& other);
};

#endif
