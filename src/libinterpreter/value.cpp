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

void Value::add(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
 } else {
   switch (type) {
      case Type::INT: {
        value.ival += other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::sub(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT: {
        value.ival -= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::mul(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT: {
        value.ival *= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::div(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT: {
        value.ival /= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::mod(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT: {
        value.ival %= other.value.ival;
        break;
      }
      default: assert(0);
    }
  }
}

void Value::rat_div(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    assert(0);
  }
}

void Value::eq(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    value.bval = type == other.type;
    type = Type::BOOLEAN;
  } else {
    switch (type) {
      case Type::INT: {
        value.bval = value.ival == other.value.ival;
        break;
      }
      case Type::BOOLEAN: {
        value.bval = value.bval == other.value.bval;
        break;
      }
      case Type::UNDEF:
        if (other.type == Type::UNDEF) {
          value.bval = true;
        } else {
          value.bval = false;
        }
        type = Type::BOOLEAN;
        break;
      default: assert(0);
    }
  }
}

void Value::lesser(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT:
        value.bval = value.ival < other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::greater(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT:
        value.bval = value.ival > other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::lessereq(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT:
        value.bval = value.ival <= other.value.ival;
        break;
      default: assert(0);
    }
  }
}

void Value::greatereq(const Value& other) {
  if (type == Type::UNDEF || other.type == Type::UNDEF) {
    type = Type::UNDEF;
  } else {
    switch (type) {
      case Type::INT:
        value.bval = value.ival >= other.value.ival;
        break;
      default: assert(0);
    }
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
