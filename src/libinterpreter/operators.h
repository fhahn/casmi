#ifndef CASMI_LIBINTERPRETER_OPERATORS_H
#define CASMI_LIBINTERPRETER_OPERATORS_H

#include "libsyntax/ast.h"

#include "libinterpreter/value.h"

namespace operators {
  const Value dispatch(Expression::Operation op, const Value& lhs, const Value& rhs);

  const Value add(const Value& lhs, const Value& rhs);
  const Value sub(const Value& lhs, const Value& rhs);
  const Value mul(const Value& lhs, const Value& rhs);
  const Value div(const Value& lhs, const Value& rhs);
  const Value mod(const Value& lhs, const Value& rhs);
  const Value rat_div(const Value& lhs, const Value& rhs);

  const Value eq(const Value& lhs, const Value& rhs);
  const Value neq(const Value& lhs, const Value& rhs);

  const Value and_(const Value& lhs, const Value& rhs);
  const Value or_(const Value& lhs, const Value& rhs);
  const Value xor_(const Value& lhs, const Value& rhs);
  const Value not_(const Value& lhs);

  const Value lesser(const Value& lhs, const Value& rhs);
  const Value greater(const Value& lhs, const Value& rhs);
  const Value lessereq(const Value& lhs, const Value& rhs);
  const Value greatereq(const Value& lhs, const Value& rhs);
};

#endif
