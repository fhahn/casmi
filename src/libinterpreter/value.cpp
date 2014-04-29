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

Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}


Value& Value::operator=(const Value& other) {
  value = other.value;
  type = other.type;
  return *this;
}


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
        type = t;
        value.list = reinterpret_cast<List*>(u->value);
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
    case TypeType::TUPLE: 
    case TypeType::TUPLE_OR_LIST: 
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
    case TypeType::TUPLE:
    case TypeType::TUPLE_OR_LIST:
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
      case TypeType::TUPLE:
      case TypeType::TUPLE_OR_LIST:
      case TypeType::LIST: return *v1.value.list == *v2.value.list;
      default: assert(0);
    }
  }
}


List::List(ListType t) : list_type(t) {}


void List::const_iterator::do_init(const List *ptr) {
  pos = 0;
  if (!ptr) {
    temp = nullptr;
    perm = nullptr;
    return;
  }

  if (ptr->is_temp()) {
    temp = reinterpret_cast<const TempList*>(ptr);
    perm = nullptr;
    if (temp->changes.size() == 0) {
      if (temp->skip > 0) {
        size_t to_skip = temp->skip+1;
        const_iterator res(*this);
        for (size_t i=0; i < to_skip; i++) {
          next();
        }
      } else {
        temp = nullptr;
      }
    }
  } else {
    perm = reinterpret_cast<const PermList*>(ptr);
    if (perm->values.size() == 0) {
      perm = nullptr;
    }
    temp = nullptr;
  }
}

List::const_iterator::const_iterator(const List *ptr) {
  do_init(ptr);
}

List::const_iterator::const_iterator(const self_type& other) : perm(other.perm), temp(other.temp), pos(other.pos) { }

List::const_iterator::self_type List::const_iterator::operator++() {
  next();
  return *this;
}

// Advancing an invalid iterator does not do anything
void List::const_iterator::next() {
  if (temp) {
    if ((1+pos) < temp->changes.size()) {
      pos += 1;
    } else {
      do_init(temp->right);
    }
  } else if (perm) {
    if ((1+pos) < perm->values.size()) {
      pos += 1;
    } else {
      do_init(nullptr);
    }
  } else {
    //assert(0);
  }
}

List::const_iterator::self_type List::const_iterator::operator++(int) {
  self_type copy(*this);
  operator++();
  return copy;
}

const Value& List::const_iterator::operator*() {
  if (temp) {
    return temp->changes[pos];
  } else if (perm) {
    return perm->values[pos];
  } else {
    assert(0);
  }
}

// all iterators that are not invalid (temp = perm = nullptr and pos = 0)
// are equal; a valid and an invalid iterator are _NOT_ equal
bool List::const_iterator::operator==(const self_type& rhs) const {
  if (!temp && !perm) {
    return !rhs.temp && !rhs.perm;
  } else {
    return rhs.temp || rhs.perm;
  }
}

bool List::const_iterator::operator!=(const self_type& rhs) const {
  return !(*this == rhs);
}

List::const_iterator List::begin() const {
  return const_iterator(this);
}

List::const_iterator List::end() const {
  return const_iterator(nullptr);
}


bool List::operator==(const List& other) const {
  auto iter1 = begin();
  auto iter2 = other.begin();

  while (iter1 != end() && iter2 != end()) {
    if (!value_eq(*iter1, *iter2)) {
      return false;
    }
    iter1++;
    iter2++;
  }

  if (iter1 == end() && iter2 == end()) {
    return true;
  } else {
    return false;
  }
}

bool List::operator!=(const List& other) const {
  return ! (*this == other);
}

bool List::is_perm() const {
  return list_type == ListType::PERM;
}

bool List::is_temp() const {
  return list_type == ListType::TEMP;
}

TempList::TempList() : List(ListType::TEMP), right(nullptr), changes(), skip(0) {}

const std::string TempList::to_str() const {
  std::string res = "";
  for (const Value& v : changes) {
    res += v.to_str() + ", ";
  }
  if (right != nullptr) {
    res += right->to_str();
  }
  return res;
}

Value TempList::at(size_t i) const {
  if (i == 0) {
    throw RuntimeException("Array access with index 0 not supported");
  }
  if ((i-1) < changes.size()) {
    return changes[i-1];
  } else {
    return Value();
  }
}


PermList::PermList() : List(ListType::PERM), values() {}

const std::string PermList::to_str() const {
  std::string res = "";

  for (const Value& v : values) {
    res += v.to_str() + ", ";
  }
  return res;
}

Value PermList::at(size_t i) const {
  if (i == 0) {
    throw RuntimeException("Array access with index 0 not supported");
  }

  if ((i-1) < values.size()) {
    return values[i-1];
  } else {
    return Value();
  }
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
      case TypeType::TUPLE: 
      case TypeType::TUPLE_OR_LIST: 
      case TypeType::LIST: 
        if (key.value.list->list_type == List::ListType::TEMP) {
          return (uint64_t) temp_list_hasher(*reinterpret_cast<TempList*>(key.value.list));
        } else {
          return (uint64_t) perm_list_hasher(*reinterpret_cast<PermList*>(key.value.list));
        }
      default: throw RuntimeException("Unsupported type in std::hash<Value>()");
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
