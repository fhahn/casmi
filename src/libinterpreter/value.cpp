#include <utility>

#include "libinterpreter/value.h"

Value::Value() : type(Type::UNDEF) {}

Value::Value(INT_T ival) : type(Type::INT) {
  value.ival = ival;
}

Value::Value(FLOAT_T fval) : type(Type::FLOAT) {
  value.fval = fval;
}

Value::Value(bool bval) : type(Type::BOOL) {
  value.bval = bval;
}

Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}
