#include <assert.h>
#include <utility>

#include "libinterpreter/value.h"
#include "libutil/exceptions.h"

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

Value::Value(Type t, std::vector<Value>* list) : type(t) {
  value.list = list;
}

Value::Value(Value& other) : type(other.type), value(other.value) {}

Value::Value(const Value& other) : type(other.type), value(other.value) {}

//Value::Value(Value&& other) : type(std::move(other.type)), value(other.value) {}

Value::Value(Type t, casm_update* u) {
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
    default: throw RuntimeException("Unsupported tye in apply");
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
  if (type == TypeType::UNDEF || other.type == TypeType::UNDEF) {
    value.bval = type == other.type;
  } else {
    switch (type.t) {
      case TypeType::INT: {
        value.bval = value.ival == other.value.ival;
        break;
      }
      case TypeType::BOOLEAN: {
        value.bval = value.bval == other.value.bval;
        break;
      }
      case TypeType::UNDEF:
        if (other.type == TypeType::UNDEF) {
          value.bval = true;
        } else {
          value.bval = false;
        }
        break;
      case TypeType::STRING:
        value.bval = *value.string == *other.value.string;
        break;
      default: assert(0);
    }
  }
  type = TypeType::BOOLEAN;
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
    default: throw RuntimeException("Unsupported type in Value.to_uint64_t");
  }
}


std::string Value::to_str() const {
  switch (type.t) {
    case TypeType::INT:
      return std::move(std::to_string(value.ival));
    case TypeType::SELF:
      return std::move("self");
    case TypeType::UNDEF: // are UNDEF and SELF the same here?
      return std::move("undef");
    case TypeType::RULEREF: 
      return std::move("@"+value.rule->name);
    case TypeType::STRING:
      return *value.string;
    default: throw RuntimeException("Unsupported type in Value.to_str() "+type.to_str());
  }
}
