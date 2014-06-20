#include <assert.h>
#include <utility>
#include <sstream>

#include "libinterpreter/operators.h"
#include "libinterpreter/execution_context.h"
#include "libinterpreter/symbolic.h"

#define HANDLE_SYMBOLIC_OR_UNDEF(lhs, rhs)                                   \
  if (lhs.is_undef() || rhs.is_undef()) {                                    \
    return value_t();                                                          \
  } else if (lhs.is_symbolic() || rhs.is_symbolic()) {                       \
    /* TODO cleanup symbols */                                               \
    return value_t(new symbol_t(symbolic::next_symbol_id()));                  \
  }                                                                          \

#define CREATE_NUMERICAL_OPERATION(op, lhs, rhs)  {                          \
  HANDLE_SYMBOLIC_OR_UNDEF(lhs, rhs)                                         \
  switch (lhs.type) {                                                        \
    case TypeType::INT:                                                      \
      return std::move(value_t(lhs.value.integer op rhs.value.integer));             \
    case TypeType::FLOAT:                                                    \
      return std::move(value_t(lhs.value.float_ op rhs.value.float_));             \
    case TypeType::RATIONAL:                                                 \
      return std::move(value_t(&(*lhs.value.rat op *rhs.value.rat)));          \
    default: FAILURE();                                                      \
  }                                                                          \
}

#define CREATE_NUMERICAL_CMP_OPERATION(op, lhs, rhs)  {                      \
  HANDLE_SYMBOLIC_OR_UNDEF(lhs, rhs)                                         \
                                                                             \
  switch (lhs.type) {                                                        \
    case TypeType::INT:                                                      \
      return std::move(value_t(lhs.value.integer op rhs.value.integer));             \
    case TypeType::FLOAT:                                                    \
      return std::move(value_t(lhs.value.float_ op rhs.value.float_));             \
    default: FAILURE();                                                      \
  }                                                                          \
}

#define CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs) {                         \
 if (lhs.is_symbolic() && !rhs.is_undef()) {                                 \
    return value_t(new symbol_t(symbolic::next_symbol_id(),                    \
                              new symbolic_condition(new value_t(lhs),         \
                                                     new value_t(rhs),         \
                                                      op)));                 \
 }                                                                           \
 if (rhs.is_symbolic() && !lhs.is_undef()) {                                 \
    return value_t(new symbol_t(symbolic::next_symbol_id(),                    \
                              new symbolic_condition(new value_t(lhs),         \
                                                     new value_t(rhs),         \
                                                      op)));                 \
  }                                                                          \
}

const value_t operators::dispatch(ExpressionOperation op, const value_t& lhs, const value_t& rhs) {
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
      if (lhs.is_symbolic() && rhs.is_symbolic() && lhs == rhs) {
        return value_t(false);
      }

      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::lesser(lhs, rhs));

    case ExpressionOperation::GREATER:
      if (lhs.is_symbolic() && rhs.is_symbolic() && lhs == rhs) {
        return value_t(false);
      }

      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::greater(lhs, rhs));

    case ExpressionOperation::LESSEREQ:
      if (lhs.is_symbolic() && rhs.is_symbolic() && lhs == rhs) {
        return value_t(true);
      }

      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::lessereq(lhs, rhs));

    case ExpressionOperation::GREATEREQ:
      if (lhs.is_symbolic() && rhs.is_symbolic() && lhs == rhs) {
        return value_t(true);
      }

      CHECK_SYMBOLIC_CMP_OPERATION(op, lhs, rhs);
      return std::move(operators::greatereq(lhs, rhs));

    default: FAILURE();
  }

}

const value_t operators::add(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_OPERATION(+, lhs, rhs);
}

const value_t operators::sub(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_OPERATION(-, lhs, rhs);
}

const value_t operators::mul(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_OPERATION(*, lhs, rhs);
}

const value_t operators::div(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_OPERATION(/, lhs, rhs);
}

const value_t operators::mod(const value_t& lhs, const value_t& rhs) {
  HANDLE_SYMBOLIC_OR_UNDEF(lhs, rhs)
  if (lhs.type == TypeType::INT) {
    return std::move(value_t(lhs.value.integer % rhs.value.integer));
  }
  return std::move(value_t());
}

const value_t operators::rat_div(const value_t& lhs, const value_t& rhs) {
  HANDLE_SYMBOLIC_OR_UNDEF(lhs, rhs)

  switch (lhs.type) {
    case TypeType::INT: {
      rational_t *result = (rational_t*) pp_mem_alloc(
        &(ExecutionContext::value_stack), sizeof(rational_t)
      );
      result->numerator = lhs.value.integer;
      result->denominator = rhs.value.integer;
      return std::move(value_t(result));
    }
    default: FAILURE();
  }
}

const value_t operators::eq(const value_t& lhs, const value_t& rhs) {
  return std::move(value_t(lhs == rhs));
}

const value_t operators::and_(const value_t& lhs, const value_t& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return value_t();
  }
  return std::move(value_t(lhs.value.boolean && rhs.value.boolean));  
}

const value_t operators::or_(const value_t& lhs, const value_t& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return value_t();
  }
  return std::move(value_t(lhs.value.boolean || rhs.value.boolean));  
}

const value_t operators::xor_(const value_t& lhs, const value_t& rhs) {
  if (lhs.is_undef() || rhs.is_undef()) {
    return value_t();
  }
  return std::move(value_t((bool)(lhs.value.boolean ^ rhs.value.boolean)));  
}

const value_t operators::not_(const value_t& lhs) {
  if (lhs.is_undef()) {
    return value_t();
  }
  return std::move(value_t(!lhs.value.boolean));  
}

const value_t operators::neq(const value_t& lhs, const value_t& rhs) {
  return std::move(value_t(lhs != rhs));
}

const value_t operators::lesser(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(<, lhs, rhs);
}

const value_t operators::greater(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(>, lhs, rhs);
}

const value_t operators::lessereq(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(<=, lhs, rhs);
}

const value_t operators::greatereq(const value_t& lhs, const value_t& rhs) {
  CREATE_NUMERICAL_CMP_OPERATION(>=, lhs, rhs);
}
