#ifndef CASMI_LIBINTERPRETER_OPERATORS_H
#define CASMI_LIBINTERPRETER_OPERATORS_H

#include "libsyntax/ast.h"

#include "libinterpreter/value.h"

namespace operators {
  const value_t dispatch(ExpressionOperation op, const value_t& lhs, const value_t& rhs);

  const value_t add(const value_t& lhs, const value_t& rhs);
  const value_t sub(const value_t& lhs, const value_t& rhs);
  const value_t mul(const value_t& lhs, const value_t& rhs);
  const value_t div(const value_t& lhs, const value_t& rhs);
  const value_t mod(const value_t& lhs, const value_t& rhs);
  const value_t rat_div(const value_t& lhs, const value_t& rhs);

  const value_t eq(const value_t& lhs, const value_t& rhs);
  const value_t neq(const value_t& lhs, const value_t& rhs);

  const value_t and_(const value_t& lhs, const value_t& rhs);
  const value_t or_(const value_t& lhs, const value_t& rhs);
  const value_t xor_(const value_t& lhs, const value_t& rhs);
  const value_t not_(const value_t& lhs);

  const value_t lesser(const value_t& lhs, const value_t& rhs);
  const value_t greater(const value_t& lhs, const value_t& rhs);
  const value_t lessereq(const value_t& lhs, const value_t& rhs);
  const value_t greatereq(const value_t& lhs, const value_t& rhs);
};

#endif
