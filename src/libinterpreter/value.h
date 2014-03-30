#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H

#include "libparse/types.h"

class Value {
  public:
    const Type type;
    union {
      INT_T ival;
      FLOAT_T fval;
    } value;

    Value();
    Value(INT_T ival);
    Value(FLOAT_T fval);
    Value(Value&& other);
};

#endif
