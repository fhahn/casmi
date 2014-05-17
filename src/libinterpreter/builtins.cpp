#include <cmath>
#include <cassert>
#include <sstream>

#include "libinterpreter/builtins.h"

const Value builtins::dispatch(BuiltinAtom::Id atom_id,  ExecutionContext& ctxt,
                               const std::vector<Value>& arguments) {
  switch (atom_id) {
    case BuiltinAtom::Id::POW:
      return std::move(pow(arguments[0], arguments[1]));

    case BuiltinAtom::Id::HEX:
      return std::move(hex(arguments[0]));

    case BuiltinAtom::Id::NTH:
      return std::move(nth(arguments[0], arguments[1]));

    case BuiltinAtom::Id::APP:
      return std::move(app(ctxt, arguments[0], arguments[1]));

    case BuiltinAtom::Id::CONS:
      return std::move(cons(ctxt, arguments[0], arguments[1]));

    case BuiltinAtom::Id::TAIL:
      return std::move(tail(ctxt, arguments[0]));

    case BuiltinAtom::Id::LEN:
      return std::move(len(arguments[0]));

    case BuiltinAtom::Id::PEEK:
      return std::move(peek(arguments[0]));

    case BuiltinAtom::Id::BOOLEAN2INT:
      return std::move(boolean2int(arguments[0]));

    case BuiltinAtom::Id::INT2BOOLEAN:
      return std::move(int2boolean(arguments[0]));

    case BuiltinAtom::Id::ENUM2INT:
      return std::move(enum2int(arguments[0]));

    case BuiltinAtom::Id::ASINT:
      return std::move(asint(arguments[0]));

    case BuiltinAtom::Id::ASFLOAT:
      return std::move(asfloat(arguments[0]));

    case BuiltinAtom::Id::ASRATIONAL:
      return std::move(asrational(arguments[0]));

    case BuiltinAtom::Id::SYMBOLIC:
      return std::move(symbolic(arguments[0]));

    default: return std::move(shared::dispatch(atom_id, arguments));
  }
}


const Value builtins::pow(const Value& base, const Value& power) {
  switch (base.type) {
    case TypeType::INT:
      return std::move(Value((INT_T)std::pow(base.value.ival, power.value.ival)));

    case TypeType::FLOAT:
      return std::move(Value((FLOAT_T)std::pow(base.value.fval, power.value.fval)));
    default: assert(0);
  }
}

const Value builtins::hex(const Value& arg) {
  // TODO LEAK!
  if (arg.is_undef()) {
    return std::move(Value(new std::string("undef")));
  }

  std::stringstream ss;
  if (arg.value.ival < 0) {
    ss << "-" << std::hex << (-1) * arg.value.ival;
  } else {
    ss << std::hex << arg.value.ival;
  }
  return std::move(Value(new std::string(ss.str())));
}

const Value builtins::nth(const Value& list_arg, const Value& index ) {
  if (list_arg.is_undef() || index.is_undef()) {
    return Value();
  }

  List *list = list_arg.value.list;
  List::const_iterator iter = list->begin();
  INT_T i = 1;

  while (iter != list->end() && i < index.value.ival) {
    i++;
    iter++;
  }
  if (i == index.value.ival && iter != list->end()) {
    return std::move(Value(*iter));
  } else {
    return std::move(Value());
  }
}

const Value builtins::app(ExecutionContext& ctxt, const Value& list, const Value& val) {
  // TODO LEAK
  if (list.is_undef()) {
    return std::move(Value());
  }

  List *current = list.value.list;

  while (1 == 1) {
    if (current->list_type == List::ListType::HEAD) {
      current = reinterpret_cast<HeadList*>(current)->right;
    }
    if (current->list_type == List::ListType::SKIP) {
      current = reinterpret_cast<SkipList*>(current)->bottom;
    }
    if (current->list_type == List::ListType::BOTTOM) {
      BottomList *bottom = reinterpret_cast<BottomList*>(current);
      if (bottom->tail) {
        current = bottom->tail;
      } else {
        break;
      }
    }
    if (current->list_type == List::ListType::TAIL) {
      TailList *tail = reinterpret_cast<TailList*>(current);
      if (tail->right) {
        current = tail->right;
      } else {
        break;
      }
    }
  }


  TailList *tail = new TailList(nullptr, val);
  ctxt.temp_lists.push_back(tail);

  if (current->list_type == List::ListType::TAIL) {
    reinterpret_cast<TailList*>(current)->right = tail;
  } else if (current->list_type == List::ListType::BOTTOM) {
    reinterpret_cast<BottomList*>(current)->tail = tail;
  } else {
    assert(0);
  }
  return std::move(Value(list.type, list.value.list));
}

const Value builtins::cons(ExecutionContext& ctxt, const Value& val, const Value& list) {
  // TODO LEAK
  if (list.is_undef()) {
    return std::move(Value());
  }

  HeadList *consed_list = new HeadList(list.value.list, val);
  ctxt.temp_lists.push_back(consed_list);
  return Value(list.type, consed_list);
}

const Value builtins::tail(ExecutionContext& ctxt, const Value& arg_list) {
  if (arg_list.is_undef()) {
    return std::move(Value());
  }

  List *list = arg_list.value.list;

  if (list->is_head()) {
    return std::move(Value(arg_list.type, reinterpret_cast<HeadList*>(list)->right));
  } else if (list->is_bottom()) {
    BottomList *btm = reinterpret_cast<BottomList*>(list);
    SkipList *skip = new SkipList(1, btm);
    ctxt.temp_lists.push_back(skip);
    return std::move(Value(arg_list.type, skip));
  } else {
    SkipList *old_skip = reinterpret_cast<SkipList*>(list);
    SkipList *skip = new SkipList(old_skip->skip+1, old_skip->bottom);
    ctxt.temp_lists.push_back(skip);
    return std::move(Value(arg_list.type, skip));
  }
}

const Value builtins::len(const Value& list_arg) {
  // TODO len is really slow right now, it itertes over complete list
  if (list_arg.is_undef()) {
    return std::move(Value());
  }

  List *list = list_arg.value.list;
  List::const_iterator iter = list->begin();

  size_t count = 0;

  while (iter != list->end()) {
    count++;
    iter++;
  }
  return std::move(Value((INT_T) count));
}

const Value builtins::peek(const Value& arg_list) {
  if (arg_list.is_undef()) {
    return std::move(Value());
  }

  List *list = arg_list.value.list;

  if (list->begin() != list->end()) {
    return std::move(Value(*(list->begin())));
  } else {
    return std::move(Value());
  }
}

const Value builtins::boolean2int(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  return std::move(Value((INT_T)arg.value.bval));
}

const Value builtins::int2boolean(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  return std::move(Value((bool)arg.value.ival));
}

const Value builtins::enum2int(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  return std::move(Value((INT_T)arg.value.enum_val->id));
}

const Value builtins::asint(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  switch (arg.type) {
    case TypeType::INT:
      return std::move(Value(arg.value.ival));
    case TypeType::FLOAT:
      return std::move(Value((INT_T)arg.value.fval));
    case TypeType::RATIONAL:
      return std::move(Value((INT_T)(arg.value.rat->numerator / arg.value.rat->denominator)));
    default: assert(0);
  }
}

const Value builtins::asfloat(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  switch (arg.type) {
    case TypeType::INT:
      return std::move(Value((FLOAT_T) arg.value.ival));
    case TypeType::FLOAT:
      return std::move(Value(arg.value.fval));
    case TypeType::RATIONAL:
      return std::move(Value(((FLOAT_T)arg.value.rat->numerator) / arg.value.rat->denominator));
    default: assert(0);
  }
}


void get_numerator_denominator(double x, int64_t *num, int64_t *denom) {
  // thanks to
  // http://stackoverflow.com/a/96035/781502
  uint64_t m[2][2];
  double startx = x;
  int64_t maxden = 10000000000;
  int64_t ai;

  /* initialize matrix */
  m[0][0] = m[1][1] = 1;
  m[0][1] = m[1][0] = 0;

  /* loop finding terms until denom gets too big */
  while (m[1][0] *  ( ai = (int64_t)x ) + m[1][1] <= maxden) {
      long t;
      t = m[0][0] * ai + m[0][1];
      m[0][1] = m[0][0];
      m[0][0] = t;
      t = m[1][0] * ai + m[1][1];
      m[1][1] = m[1][0];
      m[1][0] = t;
      if(x==(double)ai) break;     // AF: division by zero
      x = 1/(x - (double) ai);
      if(x>(double)0x7FFFFFFF) break;  // AF: representation failure
  }

  /* now remaining x is between 0 and 1/ai */
  /* approx as either 0 or 1/m where m is max that will fit in maxden */
  /* first try zero */

  double error1 = startx - ((double) m[0][0] / (double) m[1][0]);

  *num = m[0][0];
  *denom =  m[1][0];

  /* now try other possibility */
  ai = (maxden - m[1][1]) / m[1][0];
  m[0][0] = m[0][0] * ai + m[0][1];
  m[1][0] = m[1][0] * ai + m[1][1];
  double error2 = startx - ((double) m[0][0] / (double) m[1][0]);

  if (abs(error1) > abs(error2)) {
    *num = m[0][0];
    *denom =  m[1][0];
  }
}

const Value builtins::asrational(const Value& arg) {
  if (arg.is_undef()) {
    return std::move(arg);
  }

  rational_t *result = (rational_t*) pp_mem_alloc(
      &(ExecutionContext::value_stack), sizeof(rational_t)
  );
  switch (arg.type) {
    case TypeType::INT:
      result->numerator = arg.value.ival;
      result->denominator = 1;
      return std::move(Value(result));
    case TypeType::FLOAT:
      get_numerator_denominator(arg.value.fval, &result->numerator, &result->denominator);
      return std::move(Value(result));
    case TypeType::RATIONAL:
      return std::move(Value(arg.value.rat));
    default: assert(0);
  }
}

const Value builtins::symbolic(const Value& arg) {
  if (arg.type == TypeType::SYMBOL) {
    return std::move(Value(true));
  } else {
    return std::move(Value(false));
  }
}


namespace builtins {
namespace shared {
  #include "shared_glue.h"

  IGNORE_VARIADIC_WARNINGS

  // the CASM runtime heavily depens on macros, whatever you think of it ... 
  // here we need to provide all definitions ...
  #define TRUE                    1
  #define FALSE                   0
  #define ARG(TYPE, NAME)             TYPE* NAME
  #define SARG(VAR) #VAR              " {0x%lx,%u}"
  #define PARG(TYPE, VAR)             (uint64_t)VAR->value, VAR->defined
  #define CASM_RT(FORMAT, ARGS...)        /* printf-able */
  #define CASM_INFO(FORMAT, ARGS...)      /* printf-able */


  // create concrete variants of the shareds
  #define CASM_CALL_SHARED(NAME, VALUE, ARGS...)  NAME(VALUE, ##ARGS)
  #define DEFINE_CASM_SHARED(NAME, VALUE, ARGS...) void NAME(VALUE, ##ARGS)
  #include "libcasmrt/pp_casm_shared.h"

  REENABLE_VARIADIC_WARNINGS

  const Value dispatch(BuiltinAtom::Id builtin_id, 
                       const std::vector<Value>& arguments) {
    Int ret;
    Int arg0;
    Int arg1;
    Int arg2;
    Int arg3;
    Int arg4;
    switch (builtin_id) {
      SHARED_DISPATCH
      default: assert(0);
    }

    if (ret.defined == TRUE) {
      return std::move(Value((INT_T)ret.value));
    } else {
      return std::move(Value());
    }
  }
}
}
