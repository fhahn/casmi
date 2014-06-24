#ifndef CASMI_LIBINTERPRETER_BUILTINS_H
#define CASMI_LIBINTERPRETER_BUILTINS_H

#include "libsyntax/ast.h"

#include "libinterpreter/execution_context.h"
#include "libinterpreter/value.h"
#include "libinterpreter/symbolic.h"

namespace builtins {
  const value_t dispatch(BuiltinAtom::Id atom_id, ExecutionContext& ctxt,
                         const value_t arguments[], uint16_t num_arguments);

  const value_t pow(const value_t& base, const value_t& power);
  const value_t hex(const value_t& arg);
  const value_t nth(const value_t& list_arg, const value_t& index );
  const value_t app(ExecutionContext& ctxt, const value_t& list, const value_t& val);
  const value_t cons(ExecutionContext& ctxt, const value_t& val, const value_t& list);
  const value_t tail(ExecutionContext& ctxt, const value_t& arg_list);
  const value_t len(const value_t& list_arg);
  const value_t peek(const value_t& arg_list);

  const value_t boolean2int(const value_t& arg);
  const value_t int2boolean(const value_t& arg);
  const value_t enum2int(const value_t& arg);

  const value_t asint(const value_t& arg);
  const value_t asfloat(const value_t& arg);
  const value_t asrational(const value_t& arg);

  const value_t symbolic(const value_t& arg);

  namespace shared {
    struct Int {
      uint64_t value;
      uint8_t defined;
      bool sym;
    };

    const value_t dispatch(BuiltinAtom::Id builtin_id, ExecutionContext& ctxt,
                         const value_t arguments[], uint16_t num_arguments);

  }
};

#endif
