#include <assert.h>
#include <utility>

#include "libsyntax/ast.h"

#include "libinterpreter/value.h"

Value::Value() : type(TypeType::UNDEF) {}

Value::Value(INT_T ival) : type(TypeType::INT) {
  value.ival = ival;
}

Value::Value(FLOAT_T fval) : type(TypeType::FLOAT) {
  value.fval = fval;
}

Value::Value(bool bval) : type(TypeType::BOOLEAN) {
  value.bval = bval;
}

Value::Value(RuleNode *rule) : type(TypeType::RULEREF) {
  value.rule = rule;
}

Value::Value(std::string *string) : type(TypeType::STRING) {
  value.string = string;
}

Value::Value(Type t,List *list) : type(t) {
  value.list = list;
}

Value::Value(Value& other) : type(other.type), value(other.value) {}

Value::Value(const Value& other) : type(other.type), value(other.value) {}

//Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}

Value::Value(Type t, casm_update* u) {
  if (u->defined == 0) {
    type = TypeType::UNDEF;
    return;
  }

  switch (t.t) {
    case TypeType::UNDEF:
      type = TypeType::UNDEF;
      break;
    case TypeType::RULEREF:
      if (u->value != 0) {
        type = TypeType::RULEREF; 
        value.rule = reinterpret_cast<RuleNode*>(u->value);
      } else {
        type = TypeType::UNDEF; 
      }
      break;
    case TypeType::INT:
      type = TypeType::INT;
      value.ival = (int64_t)u->value;
      break;
    case TypeType::STRING:
      if (u->value != 0) {
        type = TypeType::STRING;
        value.string = reinterpret_cast<std::string*>(u->value);
      } else {
        type = TypeType::UNDEF;
      }
      break;
    case TypeType::LIST:
      if (u->value != 0) {
        type = TypeType::LIST;
        // TODO LEAK
        value.list = new TempList();
        //reinterpret_cast<std::vector<Value>*>(u->value);
      } else {
        type = TypeType::UNDEF;
      }
      break;
    default: throw RuntimeException("Unsupported type in apply");
  }
}

void Value::add(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
 } else {
   switch (type.t) {
      case TypeType::INT: {
        value.ival += other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::sub(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT: {
        value.ival -= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::mul(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT: {
        value.ival *= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::div(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT: {
        value.ival /= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::mod(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT: {
        value.ival %= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::rat_div(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    assert(0);
  }
}

void Value::eq(const Value& other) {
}

void Value::lesser(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT:
        value.bval = value.ival < other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::greater(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT:
        value.bval = value.ival > other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::lessereq(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT:
        value.bval = value.ival <= other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::greatereq(const Value& other) {
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    type = TypeType::UNDEF;
  } else {
    switch (type.t) {
      case TypeType::INT:
        value.bval = value.ival >= other.value.ival;
        break;
      default: assert(0);
    }
  }
}


uint64_t Value::to_uint64_t() const {
  switch (type.t) {
    case TypeType::INT:
      return value.ival;
    case TypeType::SELF:
    case TypeType::UNDEF: // are UNDEF and SELF the same here?
      return 0;
    case TypeType::RULEREF:
      return (uint64_t) value.rule;
    case TypeType::STRING: 
      return (uint64_t) value.string;
    case TypeType::LIST:
      return (uint64_t) value.list;
    default: throw RuntimeException("Unsupported type in Value.to_uint64_t");
  }
}

bool Value::is_undef() const {
  return type == TypeType::UNDEF;
}

std::string Value::to_str() const {
  switch (type.t) {
    case TypeType::INT:
      return std::move(std::to_string(value.ival));
    case TypeType::FLOAT:
      return std::move(std::to_string(value.fval));
    case TypeType::SELF:
      return std::move("self");
    case TypeType::UNDEF: // are UNDEF and SELF the same here?
      return std::move("undef");
    case TypeType::RULEREF: 
      return std::move("@"+value.rule->name);
    case TypeType::STRING:
      return *value.string;
    case TypeType::LIST: {
      if (value.list->list_type == List::ListType::TEMP) {
        return "[" + reinterpret_cast<TempList*>(value.list)->to_str() +"]";
      } else {
        return "[" + reinterpret_cast<PermList*>(value.list)->to_str() +"]";
      }
    }
    case TypeType::BOOLEAN:
      if (value.bval) {
        return "true";
      } else {
        return "false";
      }
    default: throw RuntimeException("Unsupported type in Value.to_str() "+type.to_str());
  }
}

bool value_eq(const Value& v1, const Value& v2) {
  if (v1.type == TypeType::UNDEF || v2.type == TypeType::UNDEF) {
    return v1.type == v2.type;
  } else {
    switch (v1.type.t) {
      case TypeType::INT: return v1.value.ival == v2.value.ival;
      case TypeType::FLOAT: return v1.value.fval == v2.value.fval;
      case TypeType::BOOLEAN: return v1.value.bval == v2.value.bval;
      case TypeType::STRING: return *v1.value.string == *v2.value.string;
      case TypeType::LIST: {

        return true;
      }
      default: assert(0);
    }
  }
}


List::List(ListType t) : list_type(t) {}

TempList::TempList() : List(ListType::TEMP), left(nullptr), right(nullptr), changes() {}

const std::string TempList::to_str() const {
  std::string res = "";
  if (left != nullptr) {
    res += left->to_str();
  }
  for (const Value& v : changes) {
    res += v.to_str() + ", ";
  }
  if (right != nullptr) {
    res += right->to_str();
  }
  return res;
}


PermList::PermList() : List(ListType::PERM), values() {}

const std::string PermList::to_str() const {
  std::string res = "";

  for (const Value& v : values) {
    res += v.to_str() + ", ";
  }
  return res;
}

namespace std {

  std::hash<std::string> hash<Value>::str_hasher;
  std::hash<TempList> hash<Value>::temp_list_hasher;
  std::hash<PermList> hash<Value>::perm_list_hasher;

  size_t hash<Value>::operator()(const Value &key) const {
    switch (key.type.t) {
      case TypeType::INT:
        return key.value.ival;
      case TypeType::SELF:
      case TypeType::UNDEF: // are UNDEF and SELF the same here?
        return 0;
      case TypeType::RULEREF:
        return (uint64_t) key.value.rule;
      case TypeType::STRING: 
        return (uint64_t) str_hasher(*key.value.string);
      case TypeType::LIST: 
        if (key.value.list->list_type == List::ListType::TEMP) {
          return (uint64_t) temp_list_hasher(*reinterpret_cast<TempList*>(key.value.list));
        } else {
          return (uint64_t) perm_list_hasher(*reinterpret_cast<PermList*>(key.value.list));
        }
      default: throw RuntimeException("Unsupported type in Value.to_uint64_t");
    }
  }


  std::hash<Value> hash<std::vector<Value>>::hasher;
  size_t hash<std::vector<Value>>::operator()(const std::vector<Value> &key) const {
    size_t h = 0;
    for (const Value& v : key) {
      h += hasher(v);
    }
    return h;
  }

  std::hash<std::vector<Value>> hash<TempList>::list_hasher;
  size_t hash<TempList>::operator()(const TempList &key) const {
    return list_hasher(key.changes);
  }

  std::hash<std::vector<Value>> hash<PermList>::list_hasher;
  size_t hash<PermList>::operator()(const PermList &key) const {
    return list_hasher(key.values);
  }
}
