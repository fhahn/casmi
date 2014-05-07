#ifndef CASMI_LIBINTERPRETER_BUILTINS_H
#define CASMI_LIBINTERPRETER_BUILTINS_H

#include "libsyntax/ast.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"

namespace builtins {
  const Value dispatch(BuiltinAtom::Id atom_id, ExecutionContext ctxt,
                       const std::vector<Value>& arguments);

  const Value pow(const Value& base, const Value& power);
  const Value hex(const Value& arg);
  const Value nth(const Value& list_arg, const Value& index );
  const Value app(ExecutionContext& ctxt, const Value& list, const Value& val);
  const Value cons(ExecutionContext& ctxt, const Value& val, const Value& list);
  const Value tail(ExecutionContext& ctxt, const Value& arg_list);
  const Value len(const Value& list_arg);
  const Value peek(const Value& arg_list);

  const Value boolean2int(const Value& arg);
  const Value int2boolean(const Value& arg);
};

#endif
