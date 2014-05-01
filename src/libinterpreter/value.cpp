#include <assert.h>
#include <utility>
#include <sstream>

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
        return value.list->to_str();
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


List::List(ListType t) : list_type(t), usage_count(0) {}


void List::const_iterator::do_init(const List *ptr) {
  pos = 0;
  if (!ptr) {
    head = nullptr;
    bottom = nullptr;
    return;
  }

  if (ptr->is_head()) {
    head = reinterpret_cast<const HeadList*>(ptr);
    bottom = nullptr;
  } else if (ptr->is_bottom()){
    bottom = reinterpret_cast<const BottomList*>(ptr);
    if (bottom->values.size() == 0) {
      bottom = nullptr;
    }
    head = nullptr;
  } else {
    const SkipList *skip = reinterpret_cast<const SkipList*>(ptr);
    if (skip->bottom->values.size() > skip->skip) {
      bottom = skip->bottom;
      pos = skip->skip;
    } else {
      bottom = nullptr;
    }
    head = nullptr;
  }
}

List::const_iterator::const_iterator(const List *ptr) {
  do_init(ptr);
}

List::const_iterator::const_iterator(const self_type& other) : bottom(other.bottom), head(other.head), pos(other.pos) { }

List::const_iterator::self_type List::const_iterator::operator++() {
  next();
  return *this;
}

// Advancing an invalid iterator does not do anything
void List::const_iterator::next() {
  if (head) {
    do_init(head->right);
  } else if (bottom) {
    if ((1+pos) < bottom->values.size()) {
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
  if (head) {
    return head->current_head;
  } else if (bottom) {
    return bottom->values[pos];
  } else {
    assert(0);
  }
}

// all iterators that are not invalid (head = bottom = nullptr and pos = 0)
// are equal; a valid and an invalid iterator are _NOT_ equal
bool List::const_iterator::operator==(const self_type& rhs) const {
  if (!head && !bottom) {
    return !rhs.head && !rhs.bottom;
  } else {
    return rhs.head || rhs.bottom;
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

bool List::is_bottom() const {
  return list_type == ListType::BOTTOM;
}

bool List::is_head() const {
  return list_type == ListType::HEAD;
}

bool List::is_skip() const {
  return list_type == ListType::SKIP;
}

const std::string List::to_str() const {
  std::stringstream res;
  res << "[";
  for (auto iter=begin(); iter != end(); iter.next()) {
    res << (*iter).to_str() << ", ";
  }
  res << "]";
  return res.str();
}


void List::bump_usage() {
  usage_count += 1;

  if (is_bottom()) {
    return;
  }

  if (is_head()) {
    reinterpret_cast<HeadList*>(this)->right->bump_usage();
  }

  if (is_skip()) {
    reinterpret_cast<SkipList*>(this)->bottom->bump_usage();
  }
}

void List::decrease_usage() {
  usage_count -= 1;

  if (is_bottom()) {
    return;
  }

  if (is_head()) {
    reinterpret_cast<HeadList*>(this)->right->bump_usage();
  }

  if (is_skip()) {
    reinterpret_cast<SkipList*>(this)->bottom->bump_usage();
  }
}

std::vector<Value> List::collect(std::vector<Value>& vec) {

  if (is_head()) {
    HeadList* list = reinterpret_cast<HeadList*>(this);
    vec.push_back(list->current_head);
    return std::move(list->right->collect(vec));
  }

  if (is_skip()) {

    SkipList *list = reinterpret_cast<SkipList*>(this);
    if (list->bottom->usage_count <= 1) {
      for (size_t i = list->skip; i < list->bottom->values.size(); i++) {
        vec.push_back(list->bottom->values[i]);
      }
      return std::move(vec);
    } else {
      assert(0);
    }
  }

  if (is_bottom()) {
    BottomList* list = reinterpret_cast<BottomList*>(this);
    if (list->usage_count <= 1) {
      for (const Value& v : list->values) {
        vec.push_back(v);
      }
      return std::move(vec);
    } else {
      assert(0);
    }
  }
}

bool List::is_used() const {
  return usage_count > 0;
}

HeadList::HeadList(List *l, const Value& val) : List(ListType::HEAD), right(l), current_head(val) {}

BottomList::BottomList() 
  : List(ListType::BOTTOM), values() {}


BottomList::BottomList(const std::vector<Value>& vals) 
  : List(ListType::BOTTOM), values(std::move(vals)) {}

BottomList::~BottomList() {
}

SkipList::SkipList(size_t skip, BottomList *btm) : List(ListType::SKIP), skip(skip), bottom(btm) {}

namespace std {

  std::hash<std::string> hash<Value>::str_hasher;
  std::hash<HeadList> hash<Value>::head_list_hasher;
  std::hash<BottomList> hash<Value>::perm_list_hasher;

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
      case TypeType::LIST: {
        size_t h = 0; 
        for (auto iter=key.value.list->begin(); iter!=key.value.list->end(); iter++) {
          h += operator()(*iter);
        }
        return h;
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

  std::hash<Value> hash<HeadList>::hasher;
  size_t hash<HeadList>::operator()(const HeadList &key) const {
    return hasher(key.current_head);
  }

  std::hash<std::vector<Value>> hash<BottomList>::list_hasher;
  size_t hash<BottomList>::operator()(const BottomList &key) const {
    return list_hasher(key.values);
  }
}
