#include <assert.h>
#include <utility>
#include <sstream>

#include "libinterpreter/operators.h"

#define CREATE_NUMERICAL_OPERATION(op, lhs, rhs)  {                   \
  if (lhs.is_undef() || rhs.is_undef()) {                                    \
    return Value();                                                          \
  }                                                                          \
                                                                             \
  switch (lhs.type) {                                             \
    case TypeType::INT:                                           \
      return std::move(Value(lhs.value.ival op rhs.value.ival));   \
    default: assert(0);                                           \
  }                                                               \
} 

const Value operators::dispatch(Expression::Operation op, const Value& lhs, const Value& rhs) {
  switch (op) {
    case Expression::Operation::ADD:
      return std::move(operators::add(lhs, rhs));

    case Expression::Operation::SUB: 
      return std::move(operators::sub(lhs, rhs));

    case Expression::Operation::MUL:
      return std::move(operators::mul(lhs, rhs));

    case Expression::Operation::DIV:
      return std::move(operators::div(lhs, rhs));

    case Expression::Operation::MOD:
      return std::move(operators::mod(lhs, rhs));

    case Expression::Operation::RAT_DIV:
      return std::move(operators::rat_div(lhs, rhs));

    case Expression::Operation::EQ:
      return std::move(operators::eq(lhs, rhs));

    case Expression::Operation::NEQ: 
      return std::move(operators::neq(lhs, rhs));

    case Expression::Operation::LESSER:
      return std::move(operators::lesser(lhs, rhs));

    case Expression::Operation::GREATER:
      return std::move(operators::greater(lhs, rhs));

    case Expression::Operation::LESSEREQ:
      return std::move(operators::lessereq(lhs, rhs));

    case Expression::Operation::GREATEREQ:
      return std::move(operators::greatereq(lhs, rhs));

    default: assert(0);
  }

}

const Value operators::add(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(+, lhs, rhs);
}

const Value operators::sub(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(-, lhs, rhs);
}

const Value operators::mul(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(*, lhs, rhs);
}

const Value operators::div(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(/, lhs, rhs);
}

const Value operators::mod(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(%, lhs, rhs);
}

const Value operators::rat_div(const Value& lhs, const Value& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return Value();
  }
  switch (lhs.type) {
    default: assert(0);
  }
}

const Value operators::eq(const Value& lhs, const Value& rhs) {
  return std::move(Value(lhs == rhs));
}

const Value operators::neq(const Value& lhs, const Value& rhs) {
  return std::move(Value(lhs != rhs));
}

const Value operators::lesser(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(<, lhs, rhs);
}

const Value operators::greater(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(>, lhs, rhs);
}

const Value operators::lessereq(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(<=, lhs, rhs);
}

const Value operators::greatereq(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_OPERATION(>=, lhs, rhs);
}
