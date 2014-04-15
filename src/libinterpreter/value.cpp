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

Value::Value(std::string *string) : type(Type::STRING) {
  value.string = string;
}

Value::Value(Value& other) : type(other.type), value(other.value) {}

Value::Value(const Value& other) : type(other.type), value(other.value) {}

//Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}

Value::Value(Type t, casm_update* u) {
  switch (t) {
    case Type::UNDEF:
      type = Type::UNDEF;
      break;
    case Type::RULEREF:
      if (u->value != 0) {
        type = Type::RULEREF; 
        value.rule = reinterpret_cast<RuleNode*>(u->value);
      } else {
        type = Type::UNDEF; 
      }
      break;
    case Type::INT:
      type = Type::INT;
      value.ival = (int64_t)u->value;
      break;
    case Type::STRING:
      if (u->value != 0) {
        type = Type::STRING;
        value.string = reinterpret_cast<std::string*>(u->value);
      } else {
        type = Type::UNDEF;
      }
      break;
    default: throw RuntimeException("Unsupported tye in apply");
  }
}

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
        break;
      case Type::STRING:
        value.bval = *value.string == *other.value.string;
        break;
      default: assert(0);
    }
  }
  type = Type::BOOLEAN;
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
    case Type::RULEREF:
      return (uint64_t) value.rule;
    case Type::STRING: 
      return (uint64_t) value.string;
    default: throw RuntimeException("Unsupported type in Value.to_uint64_t");
  }
}


std::string Value::to_str() const {
  switch (type) {
    case Type::INT:
      return std::move(std::to_string(value.ival));
    case Type::SELF:
      return std::move("self");
    case Type::UNDEF: // are UNDEF and SELF the same here?
      return std::move("undef");
    case Type::RULEREF: 
      return std::move("@"+value.rule->name);
    case Type::STRING:
      return *value.string;
    default: throw RuntimeException("Unsupported type in Value.to_str() "+type_to_str(type));
  }
}
