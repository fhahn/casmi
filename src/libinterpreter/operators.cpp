#include <assert.h>
#include <utility>
#include <sstream>

#include "libinterpreter/operators.h"
#include "libinterpreter/execution_context.h"

#define CREATE_NUMERICAL_OPERATION(op, lhs, rhs)  {                   \
  if (lhs.is_undef() || rhs.is_undef()) {                                    \
    return Value();                                                          \
  }                                                                          \
                                                                             \
  switch (lhs.type) {                                             \
    case TypeType::INT:                                           \
      return std::move(Value(lhs.value.ival op rhs.value.ival));   \
    case TypeType::FLOAT:                                           \
      return std::move(Value(lhs.value.fval op rhs.value.fval));   \
    case TypeType::RATIONAL:                                           \
      return std::move(Value(&(*lhs.value.rat op *rhs.value.rat)));   \
    default: assert(0);                                           \
  }                                                               \
}

#define CREATE_NUMERICAL_CMP_OPERATION(op, lhs, rhs)  {                   \
  if (lhs.is_undef() || rhs.is_undef()) {                                    \
    return Value();                                                          \
  }                                                                          \
                                                                             \
  switch (lhs.type) {                                             \
    case TypeType::INT:                                           \
      return std::move(Value(lhs.value.ival op rhs.value.ival));   \
    case TypeType::FLOAT:                                           \
      return std::move(Value(lhs.value.fval op rhs.value.fval));   \
    default: assert(0);                                           \
  }                                                               \
}

#define CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs) {                         \
 if (lhs.is_symbolic() && !rhs.is_undef()) {                                 \
    return Value(new symbolic_condition(new Value(lhs),                      \
                                        new Value(rhs),                      \
                                        op));                                \
  }                                                                          \
 if (rhs.is_symbolic() && !lhs.is_undef()) {                                 \
    return Value(new symbolic_condition(new Value(lhs),                      \
                                        new Value(rhs),                      \
                                        op));                                \
  }                                                                          \
}


#define CREATE_LOGICAL_OPERATION(op, lhs, rhs)  {                   \
 \
} 

const Value operators::dispatch(ExpressionOperation op, const Value& lhs, const Value& rhs) {
  switch (op) {
    case ExpressionOperation::ADD:
      return std::move(operators::add(lhs, rhs));

    case ExpressionOperation::SUB: 
      return std::move(operators::sub(lhs, rhs));

    case ExpressionOperation::MUL:
      return std::move(operators::mul(lhs, rhs));

    case ExpressionOperation::DIV:
      return std::move(operators::div(lhs, rhs));

    case ExpressionOperation::MOD:
      return std::move(operators::mod(lhs, rhs));

    case ExpressionOperation::RAT_DIV:
      return std::move(operators::rat_div(lhs, rhs));

    case ExpressionOperation::EQ:
      if (lhs.is_symbolic() && rhs.is_symbolic()) {
        return std::move(operators::eq(lhs, rhs));
      } else {
        CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
        return std::move(operators::eq(lhs, rhs));
      }

    case ExpressionOperation::NEQ: 
      if (lhs.is_symbolic() && rhs.is_symbolic()) {
        return std::move(operators::neq(lhs, rhs));
      } else {
        CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
        return std::move(operators::neq(lhs, rhs));
      }

    case ExpressionOperation::AND: 
      return std::move(operators::and_(lhs, rhs));

    case ExpressionOperation::OR: 
      return std::move(operators::or_(lhs, rhs));

    case ExpressionOperation::XOR: 
      return std::move(operators::xor_(lhs, rhs));

    case ExpressionOperation::NOT: 
      return std::move(operators::not_(lhs));

    case ExpressionOperation::LESSER:
      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::lesser(lhs, rhs));

    case ExpressionOperation::GREATER:
      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::greater(lhs, rhs));

    case ExpressionOperation::LESSEREQ:
      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::lessereq(lhs, rhs));

    case ExpressionOperation::GREATEREQ:
      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
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
  if (lhs.is_undef() || rhs.is_undef()) {                                    \
    return Value();                                                          \
  }                 
  if (lhs.type == TypeType::INT) {
    return std::move(Value(lhs.value.ival % rhs.value.ival));
  }
  return std::move(Value());
}

const Value operators::rat_div(const Value& lhs, const Value& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return Value();
  }
  switch (lhs.type) {
    case TypeType::INT: {
      rational_t *result = (rational_t*) pp_mem_alloc(
        &(ExecutionContext::value_stack), sizeof(rational_t)
      );
      result->numerator = lhs.value.ival;
      result->denominator = rhs.value.ival;
      return std::move(Value(result));
    }
    default: assert(0);
  }
}

const Value operators::eq(const Value& lhs, const Value& rhs) {
  return std::move(Value(lhs == rhs));
}

const Value operators::and_(const Value& lhs, const Value& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return Value();
  }
  return std::move(Value(lhs.value.bval && rhs.value.bval));  
}

const Value operators::or_(const Value& lhs, const Value& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return Value();
  }
  return std::move(Value(lhs.value.bval || rhs.value.bval));  
}

const Value operators::xor_(const Value& lhs, const Value& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return Value();
  }
  return std::move(Value((bool)(lhs.value.bval ^ rhs.value.bval)));  
}

const Value operators::not_(const Value& lhs) {
  if (lhs.is_undef()) {
    return Value();
  }
  return std::move(Value(!lhs.value.bval));  
}

const Value operators::neq(const Value& lhs, const Value& rhs) {
  return std::move(Value(lhs != rhs));
}

const Value operators::lesser(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(<, lhs, rhs);
}

const Value operators::greater(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(>, lhs, rhs);
}

const Value operators::lessereq(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(<=, lhs, rhs);
}

const Value operators::greatereq(const Value& lhs, const Value& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(>=, lhs, rhs);
}
