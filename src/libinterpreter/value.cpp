#include <utility>

#include "libinterpreter/value.h"
#include "libutil/exceptions.h"

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


uint64_t Value::to_uint64_t() const {
  switch (type) {
    case Type::INT:
      return value.ival;
    case Type::SELF:
    case Type::UNDEF: // are UNDEF and SELF the same here?
      return 0;
    default: throw RuntimeException("Unsupported type in update");
  }
}
