#include <cassert>
#include <sstream>

#include "libinterpreter/builtins.h"

const Value builtins::dispatch(BuiltinAtom::Id atom_id,  ExecutionContext ctxt,
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

    default: assert(0);
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
  if (list_arg.is_undef()) {
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
