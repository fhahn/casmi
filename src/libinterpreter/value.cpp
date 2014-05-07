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

Value::Value(const Type &t, List *list) : type(t.t) {
  value.list = list;
}

Value::Value(Value& other) : type(other.type), value(other.value) {}

Value::Value(const Value& other) : type(other.type), value(other.value) {}

Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}

Value::Value(TypeType t, casm_update* u) {
  if (u->defined == 0) {
    type = TypeType::UNDEF;
    return;
  }

  switch (t) {
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
    case TypeType::ENUM:
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
        value.list = reinterpret_cast<List*>(u->value);
      } else {
        type = TypeType::UNDEF;
      }
      break;
    default: throw RuntimeException("Unsupported type in apply");
  }
}

Value& Value::operator=(const Value& other) {
  value = other.value;
  type = other.type;
  return *this;
}

bool Value::operator==(const Value &other) const {
  if (is_undef() || other.is_undef()) {
    return is_undef() && other.is_undef();
  }

  switch (type) {
    case TypeType::INT: return value.ival == other.value.ival;
    case TypeType::FLOAT: return value.fval == other.value.fval;
    case TypeType::BOOLEAN: return value.bval == other.value.bval;
    case TypeType::ENUM: // is this save enough?
    case TypeType::STRING: return *value.string == *other.value.string;
    case TypeType::TUPLE:
    case TypeType::TUPLE_OR_LIST:
    case TypeType::LIST: return *value.list == *other.value.list;
    default: assert(0);
  }
}

bool Value::operator!=(const Value &other) const {
  return !operator==(other);
}

uint64_t Value::to_uint64_t() const {
  switch (type) {
    case TypeType::INT:
      return value.ival;
    case TypeType::SELF:
    case TypeType::UNDEF: // are UNDEF and SELF the same here?
      return 0;
    case TypeType::RULEREF:
      return (uint64_t) value.rule;
    case TypeType::ENUM:
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
  switch (type) {
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
    case TypeType::ENUM:
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
    default: throw RuntimeException("Unsupported type in Value.to_str() ");
  }
}


List::List(ListType t) : list_type(t) {}


void List::const_iterator::do_init(const List *ptr) {
  pos = 0;
  if (!ptr) {
    head = nullptr;
    bottom = nullptr;
    tail = nullptr;
    return;
  }

  if (ptr->is_head()) {
    head = reinterpret_cast<const HeadList*>(ptr);
    bottom = nullptr;
    tail = nullptr;
  } else if (ptr->is_bottom()){
    bottom = reinterpret_cast<const BottomList*>(ptr);
    pos = bottom->values.size() - 1;
    if (bottom->values.size() == 0) {
      do_init(bottom->tail);
    } else {
      head = nullptr;
      tail = nullptr;
    }
  } else if (ptr->is_skip()){
    bottom = nullptr;
    const SkipList *skip = reinterpret_cast<const SkipList*>(ptr);
    if (skip->bottom->values.size() > skip->skip) {
      bottom = skip->bottom;
      pos = skip->bottom->values.size() - skip->skip - 1;
      tail = nullptr;
    } else {
      if (skip->bottom->tail) {
        TailList *current = skip->bottom->tail;
        size_t i;
        size_t additional_skip =  skip->skip - skip->bottom->values.size();
        for (i=0; current && i < additional_skip; i++) {
          current = current->right;
        }

        if (i == additional_skip) {
          tail = current;
        } else {
          tail = nullptr;
        }
      } else {
        tail = nullptr;
      }
    }
    head = nullptr;
  } else if (ptr->is_tail()) {
    tail = reinterpret_cast<const TailList*>(ptr);
    head = nullptr;
    bottom = nullptr;
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
    if (pos > 0) {
      pos -= 1;
    } else {
      do_init(bottom->tail);
    }
  } else if (tail) {
      do_init(tail->right);
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
  } else if (tail) {
    return tail->current_tail;
  } else {
    assert(0);
  }
}

// all iterators that are not invalid (head = bottom = nullptr and pos = 0)
// are equal; a valid and an invalid iterator are _NOT_ equal
bool List::const_iterator::operator==(const self_type& rhs) const {
  if (!head && !bottom && !tail) {
    return !rhs.head && !rhs.bottom && !rhs.tail;
  } else {
    return rhs.head || rhs.bottom || rhs.tail;
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
    if (*iter1 != *iter2) {
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

bool List::is_tail() const {
  return list_type == ListType::TAIL;
}

const std::string List::to_str() const {
  std::stringstream res;
  bool add_comma = false;
  res << "[ ";
  for (auto iter=begin(); iter != end(); iter.next()) {
    if (add_comma) {
      res << ", ";
    }
    if ((*iter).type == TypeType::STRING) {
      res << "\"" << (*iter).to_str() << "\"";
    } else {
      res << (*iter).to_str();
    }
    add_comma = true;
  }
  res << " ]";
  return res.str();
}


void List::bump_usage() {
  if (is_bottom()) {
    reinterpret_cast<BottomList*>(this)->usage_count += 1;
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
  if (is_bottom()) {
    reinterpret_cast<BottomList*>(this)->usage_count -= 1;
    return;
  }

  if (is_head()) {
    reinterpret_cast<HeadList*>(this)->right->bump_usage();
  }

  if (is_skip()) {
    reinterpret_cast<SkipList*>(this)->bottom->bump_usage();
  }
}

BottomList* List::collect() {
  if (is_head()) {
    HeadList* list = reinterpret_cast<HeadList*>(this);
    BottomList *result = list->right->collect();
    result->values.push_back(std::move(list->current_head));
    return result;
  }

  if (is_skip()) {
    SkipList *list = reinterpret_cast<SkipList*>(this);
    BottomList *result = list->bottom->collect();
    for (size_t i=list->skip; i > 0; i--) {
      result->values.pop_back();
    }
    return result;
  }

  if (is_bottom()) {
    BottomList* list = reinterpret_cast<BottomList*>(this);
    if (list->usage_count <= 1) {
      list->usage_count = 1;
      if (list->tail) {
        list->tail->collect(list->values);
        list->tail = nullptr;
      }
      return list;
    } else {
      BottomList *copy = new BottomList();
      copy->usage_count = 1;
      copy->values = list->values;
      list->usage_count -= 1;
      copy->allocated_in_collect = true;

      if (list->tail) {
        list->tail->collect(copy->values);
        list->tail = nullptr;
      }
      return copy;
    }
  }
}

HeadList::HeadList(List *l, const Value& val) : List(ListType::HEAD), right(l), current_head(val) {}

TailList::TailList(TailList *l, const Value& val) : List(ListType::TAIL), right(l), current_tail(val) {}

void TailList::collect(std::vector<Value>& collect_to) {
  collect_to.insert(collect_to.begin(), current_tail);
  if (right) {
    right->collect(collect_to);
  }
}

BottomList::BottomList() 
  : List(ListType::BOTTOM), usage_count(0), values(), tail(nullptr) {}


BottomList::BottomList(const std::vector<Value>& vals) 
  : List(ListType::BOTTOM), usage_count(0), values(std::move(vals)), tail(nullptr) {}

BottomList::~BottomList() {
}


bool BottomList::is_used() const {
  return usage_count > 0;
}

bool BottomList::check_allocated_and_set_to_false() {
  if (allocated_in_collect) {
    allocated_in_collect = false;
    return true;
  }
  return false;
}

SkipList::SkipList(size_t skip, BottomList *btm) : List(ListType::SKIP), skip(skip), bottom(btm) {}

namespace std {

  std::hash<std::string> hash<Value>::str_hasher;
  std::hash<HeadList> hash<Value>::head_list_hasher;
  std::hash<BottomList> hash<Value>::perm_list_hasher;

  size_t hash<Value>::operator()(const Value &key) const {
    switch (key.type) {
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
