#include <assert.h>
#include <utility>

#include "libinterpreter/value.h"
#include "libutil/exceptions.h"

Value::Value() : type(Type::UNDEF) {}

Value::Value(INT_T ival) : type(Type::INT) {
  value.ival = ival;
}

Value::Value(FLOAT_T fval) : type(Type::FLOAT) {
  value.fval = fval;
}

Value::Value(bool bval) : type(Type::BOOLEAN) {
  value.bval = bval;
}

Value::Value(RuleNode *rule) : type(Type::RULEREF) {
  value.rule = rule;
}

Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}

void Value::add(Value& other) {
  switch (type) {
    case Type::INT: {
      value.ival += other.value.ival;
      break;
    }
    default: assert(0);
  }
}

void Value::sub(Value& other) {
  switch (type) {
    case Type::INT: {
      value.ival -= other.value.ival;
      break;
    }
    default: assert(0);
  }
}

void Value::mul(Value& other) {
  switch (type) {
    case Type::INT: {
      value.ival *= other.value.ival;
      break;
    }
    default: assert(0);
  }
}

void Value::div(Value& other) {
  switch (type) {
    case Type::INT: {
      value.ival /= other.value.ival;
      break;
    }
    default: assert(0);
  }
}

void Value::mod(Value& other) {
  switch (type) {
    case Type::INT: {
      value.ival %= other.value.ival;
      break;
    }
    default: assert(0);
  }
}
uint64_t Value::to_uint64_t() const {
  switch (type) {
    case Type::INT:
      return value.ival;
    case Type::SELF:
    case Type::UNDEF: // are UNDEF and SELF the same here?
      return 0;
    case Type::RULEREF: return (uint64_t) value.rule;
    default: throw RuntimeException("Unsupported type in Value.to_uint64_t");
  }
}
