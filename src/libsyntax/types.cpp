#include <assert.h>

#include <string>
#include <map>
#include <sstream>

#include "macros.h"

#include "libsyntax/types.h"


Type::Type(const std::string& type_name, std::vector<Type*>& st) :
    unify_with_left(nullptr), unify_with_right(nullptr), constraints(),
    subtypes(st), enum_name() {

  if (type_name == "List") {
    t = TypeType::LIST;

    if (subtypes.size() == 0) {
      subtypes.push_back(new Type(TypeType::UNKNOWN));
    } else if (subtypes.size() > 1) {
      t = TypeType::INVALID;
    }
  } else if (type_name == "Tuple") {
    if (subtypes.size() > 0) {
      t = TypeType::TUPLE;
    } else {
      t = TypeType::INVALID;
    }
  }
}

Type::Type(TypeType typ, std::vector<Type*>& st) : t(typ), unify_with_left(nullptr),
    unify_with_right(nullptr), constraints(), subtypes(st), enum_name() {

  if (t == TypeType::LIST) {
    t = TypeType::LIST;
    if (subtypes.size() == 0) {
      subtypes.push_back(new Type(TypeType::UNKNOWN));
    } else if (subtypes.size() > 1) {
      t = TypeType::INVALID;
    }
  } else if (t != TypeType::TUPLE_OR_LIST && t != TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
}

Type::Type(TypeType typ, Type *subtype) : t(typ), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), subtypes(), enum_name() {
  if (t != TypeType::LIST && t != TypeType::TUPLE_OR_LIST) {
    t = TypeType::INVALID;
  }
  subtypes.push_back(subtype);
}

Type::Type() : t(TypeType::INVALID), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), subtypes(), enum_name() {}

Type::Type(TypeType t) : t(t), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), subtypes(), enum_name() {
  if (t == TypeType::LIST || t == TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
}

Type::Type(Type *other) : t(other->t), unify_with_left(nullptr),
    unify_with_right(nullptr), constraints(), subtypes(), enum_name(other->enum_name)  {
  for (Type* t : other->subtypes) {
    subtypes.push_back(new Type(t));
  }
}


Type::Type(const Type& other) : t(other.t), unify_with_left(nullptr),
    unify_with_right(nullptr), constraints(), subtypes(other.subtypes), enum_name(other.enum_name)  {
}

Type::Type(TypeType enum_type, const std::string& name) : unify_with_left(nullptr), unify_with_right(nullptr), constraints(), subtypes(), enum_name(std::move(name))  {
  if (enum_type == TypeType::ENUM) {
    t = enum_type;
  } else {
    t = TypeType::INVALID;
  }
}

Type::Type(const std::string& type_name) : unify_with_left(nullptr),
    unify_with_right(nullptr), constraints(), subtypes() {
  if (type_name == "Int") { t = TypeType::INT; }
  else if (type_name == "Float") { t = TypeType::FLOAT; }
  else if (type_name == "Rational") { t = TypeType::RATIONAL; }
  else if (type_name == "Undef") { t = TypeType::UNDEF; }
  else if (type_name == "Boolean") { t = TypeType::BOOLEAN; }
  else if (type_name == "RuleRef") { t = TypeType::RULEREF; }
  else if (type_name == "String") { t = TypeType::STRING; }
  else {
    // if the string does not match any known type assume enum;
    // make sure enum exists during typechecking
    t = TypeType::ENUM;
    enum_name = type_name;
  }
}

bool Type::eq(const Type& other) const {
  if (t != other.t) {
    return false;
  }

  if (subtypes.size() != other.subtypes.size()) {
    return false;
  }

  for (uint32_t i=0; i < subtypes.size(); i++) {
    if (*(subtypes[i]) != *(other.subtypes[i])) {
      return false;
    }
  }

  return true;
}

bool Type::operator==(const Type& other) const {
  return eq(other);
}

bool Type::operator==(const TypeType other) const {
  return t == other;
}

bool Type::operator!=(const Type& other) const {
  return !eq(other);
}

const std::string Type::to_str() const {
  switch (t) {
    case TypeType::INT: return "Int";
    case TypeType::FLOAT: return "Float";
    case TypeType::RATIONAL: return "Rational";
    case TypeType::UNDEF: return "Undef";
    case TypeType::BOOLEAN: return "Boolean";
    case TypeType::RULEREF: return "RuleRef";
    case TypeType::STRING: return "String";
    case TypeType::LIST: 
      if (subtypes.size() == 0) {
        return "List(Unknown)";
      } else {
        return "List("+subtypes[0]->to_str()+")";
      }
    case TypeType::UNKNOWN: return "Unknown";
    case TypeType::SELF: return "Self";
    case TypeType::TUPLE_OR_LIST: /*{
      std::string res = "TupleOrList (";
      for (Type* t:subtypes) {
        res += t->to_str() + ",";
      }
      return res + ")";
    } */
    case TypeType::TUPLE:{
      std::string res = "Tuple (";
      for (Type* t:subtypes) {
        res += t->to_str() + ",";
      }
      return res + ")";
    }
    case TypeType::ENUM: return enum_name;
    case TypeType::INVALID: return "Invalid";
    default: assert(0);
  }
}

std::string Type::unify_links_to_str() const {
  return unify_links_to_str_left() + " " +  unify_links_to_str_right();
}

std::string Type::unify_links_to_str_left() const {
  std::stringstream stream;
  stream << std::hex << this;
  std::string res = stream.str();

  if (unify_with_left) {
    DEBUG("UNBIF LEFT "<<this << " "<<unify_with_left);
    res = unify_with_left->unify_links_to_str_left() + "<-" + res;
  }
  return res;
}
std::string Type::unify_links_to_str_right() const {
  std::stringstream stream;
  stream << std::hex << this;
  std::string res = stream.str();

  if (unify_with_right) {
    DEBUG("UNBIF RIGHT");
    res += "->" + unify_with_right->unify_links_to_str_right(); 
  }
  return res;
}

const Type* Type::get_most_general_type() const {
  if (!is_complete()) {
    return this;
  }

  const Type *right = nullptr;
  if (unify_with_right) {
    right = unify_with_right->get_most_general_type();
    if (!right->is_complete()) {
      return right;
    }
  }

  const Type *left = this;
  if (unify_with_left) {
    left = unify_with_left->get_most_general_type();
    if (!left->is_complete()) {
      return left; 
    }
  }
  return this;
}

bool Type::unify(Type other) {
  if (t == TypeType::UNKNOWN) {
    t = other.t;
    return true;
  } else if (other.t == TypeType::UNKNOWN) {
    other.t = t;
    return true;
  }
  return t == other.t;
}

bool Type::unify_list(Type *other) {
  bool result = true;
  if(other->t == TypeType::LIST) {
    return subtypes[0]->unify(other->subtypes[0]);
  } else if(other->t == TypeType::TUPLE_OR_LIST) {
    bool internal_complete = subtypes[0]->is_complete();
    if (other->subtypes.size() > 0 && other->subtypes[0]->t != TypeType::UNKNOWN) {
      for (Type* typ : other->subtypes) {
        result = result && typ->unify(subtypes[0]);
      }
    }
    if (result) {
      other->t = TypeType::LIST;
      other->subtypes = { new Type(subtypes[0]) };
    } else {
      if (!internal_complete) {
        subtypes[0]->t = TypeType::UNKNOWN;
      }
    }
    return result;
  } else if (other->t == TypeType::TUPLE) {
    if (!is_complete()) {
      return false;
    }
    for (Type* typ : other->subtypes) {
      result = result && typ->unify(subtypes[0]);
    }
    if (result) {
      t = TypeType::TUPLE;
      subtypes = other->subtypes;
    }
    return result;
  } else if (other->t == TypeType::UNKNOWN) {
    if (other->constraints.size() > 0) {
      // constraints cover only basic types at the moment
      return false;
    }
    other->subtypes = { new Type(TypeType::UNKNOWN) };
    other->t = TypeType::LIST;
    return subtypes[0]->unify(other->subtypes[0]);
 
  }
  return false;
}

bool Type::unify_tuple(Type *other) {
  if (other->t == TypeType::TUPLE_OR_LIST || other->t == TypeType::TUPLE) {
    bool result = true;
    other->t = TypeType::TUPLE;
    if (subtypes.size() == 0 || (subtypes.size() == 1 && subtypes[0]->t == TypeType::UNKNOWN)) {
      subtypes = other->subtypes;
      return true;
    } else if (other->subtypes.size() == 0 || (other->subtypes.size() == 1 && other->subtypes[0]->t == TypeType::UNKNOWN)) {
      other->subtypes = subtypes;
      return true;
    } else if (subtypes.size() == other->subtypes.size()) {
      for (size_t i=0; i<subtypes.size(); i++) {
        result = result && subtypes[i]->unify(other->subtypes[i]);
      }
      return result;
    } 
  } else if (other->t == TypeType::UNKNOWN) {
    if (other->constraints.size() > 0) {
      // constraints cover only basic types at the moment
      return false;
    }
    other->t = t;
    other->subtypes = subtypes;
    return true;
  }

  // the case that other->t == TypeType::LIST should be covered prevoiusly by
  // unify_list
  return false;
}


bool Type::unify_tuple_or_list(Type *other) {
  if (other->t == TypeType::TUPLE_OR_LIST) {
    if (subtypes.size() == 0 || (subtypes.size() == 1 && subtypes[0]->t == TypeType::UNKNOWN)) {
      subtypes = other->subtypes;
      return true;
    } else if (other->subtypes.size() == 0 || (other->subtypes.size() == 1 && other->subtypes[0]->t == TypeType::UNKNOWN)) {
      other->subtypes = subtypes;
      return true;
    } else if (subtypes.size() == other->subtypes.size()) {
      bool result = true;
      for (size_t i=0; i<subtypes.size(); i++) {
        result = result && subtypes[i]->unify(other->subtypes[i]);
      }
      return result;
    } else {
      return false;
    }
  } else if (other->t == TypeType::UNKNOWN) {
    if (other->constraints.size() > 0) {
      // constraints cover only basic types at the moment
      return false;
    }
    other->t = t;
    other->subtypes = subtypes;
    return true;
  }
  return false;
} 

bool Type::unify_enum(Type *other) {
  if (other->t == TypeType::ENUM) {
    if (enum_name.size() == 0 && other->enum_name.size() > 0) {
      enum_name = other->enum_name;
    } else if (enum_name.size() > 0 && other->enum_name.size() == 0) {
      other->enum_name = enum_name;
    }
    return enum_name == other->enum_name;
  } else if (other->t == TypeType::UNKNOWN) {
    if (other->constraints.size() > 0) {
      // constraints cover only basic types at the moment
      return false;
    }
    other->t = TypeType::ENUM;
    other->enum_name = enum_name;
    return true;
  }
  return false;
}

bool Type::unify_nofollow(Type *other) {
  if( t == TypeType::LIST) {
    return unify_list(other);
  } else if (other->t == TypeType::LIST) {
    return other->unify_list(this);
  } else if (t == TypeType::TUPLE ) {
    return unify_tuple(other);
  } else if (other->t == TypeType::TUPLE) {
    return other->unify_tuple(this);
  } else if (t == TypeType::TUPLE_OR_LIST) {
    return unify_tuple_or_list(other);
  } else if (other->t == TypeType::TUPLE_OR_LIST) {
    return other->unify_tuple_or_list(this);
  } else if (t == TypeType::ENUM) {
    return unify_enum(other);
  } else if (other->t == TypeType::ENUM) {
    return other->unify_enum(this);
  }

  if (t != TypeType::UNKNOWN && other->t != TypeType::UNKNOWN) {
    return t == other->t;
  }

  if (other->t != TypeType::UNKNOWN) {
    t = other->t;
    bool matched_constraint = true;
    for (Type constraint : constraints) {
      if (unify(constraint)) {
        matched_constraint = true;
        break;
      } else {
        matched_constraint = false;
      }
    }

    if (!matched_constraint) {
      DEBUG("Did not match any constriants");
      return false;
    }
    return true;
  }

  if (t != TypeType::UNKNOWN) {
    other->t = t;
    bool matched_constraint = true;
    for (Type constraint : other->constraints) {
      if (unify(constraint)) {
        matched_constraint = true;
        break;
      } else {
        matched_constraint = false;
      }
    }

    if (!matched_constraint) {
      DEBUG("Did not match any constriants");
      return false;
    }
    return  true;
  }

  return true;
}

bool Type::unify_left(Type *other) {
  if (other == nullptr) {
    return true;
  }
  DEBUG("LEFFT "<< other << " "<<to_str() << " "<<other->to_str());

  
  bool result = unify_nofollow(other);
  if (other->unify_with_left != nullptr) {
    result = result && unify_nofollow(other) && unify_left(other->unify_with_left);
  }

  return result;
}

bool Type::unify_right(Type *other) {
  DEBUG("RIGHT "<<other);
  if (other == nullptr) {
    return true;
  }
  bool result = unify_nofollow(other);

  if (other->unify_with_right != nullptr) {
    result = result && unify_right(other->unify_with_right);
  }
  return result;
}

bool Type::is_unknown() const {
  if (t == TypeType::UNKNOWN) {
    return true;
  } else if (t == TypeType::TUPLE || t == TypeType::TUPLE_OR_LIST) {
    if (subtypes.size() == 0 || (subtypes.size() == 1 && subtypes[0]->t == TypeType::UNKNOWN)) {
      return true;
    }
  }
  return false;
}

bool Type::unify(Type *other) {
  DEBUG("START "<<to_str() << " "<<other->to_str());
  if (is_unknown() && other->is_unknown() ) {
    Type* left_link = this;
    while (left_link->unify_with_left != nullptr) {
      if (left_link == other) {
        return true;
      }
      left_link = left_link->unify_with_left;
    }

    Type* right_link = other;
    while (right_link->unify_with_right != nullptr) {
      if (right_link == this) {
        return true;
      }
      right_link = right_link->unify_with_right;
    }

    left_link = other;
    while (left_link->unify_with_left != nullptr) {
      if (left_link == this) {
        return true;
      }
      left_link = left_link->unify_with_left;
    }

    right_link = this;
    while (right_link->unify_with_right != nullptr) {
      if (right_link == other) {
        return true;
      }
      right_link = right_link->unify_with_right;
    }

    left_link->unify_with_left = right_link;
    right_link->unify_with_right = left_link;
    DEBUG("Link "<<this << " with " << other);
    return true;
  }

  if (unify_nofollow(other)) {
    bool result = true;
    //DEBUG("UNIFY LEFT");
    result = result && unify_left(other->unify_with_left);
    result = result && unify_left(unify_with_left);
    //DEBUG("UNIFY RIGHT");
    result = result && unify_right(other->unify_with_right);
    result = result && unify_right(unify_with_right);
    return result;
  }
  return false;
}


bool Type::is_complete() const {
  if (t == TypeType::UNKNOWN) {
    return false;
  }
  if (t == TypeType::LIST || t == TypeType::TUPLE || t == TypeType::TUPLE_OR_LIST) {
    if (subtypes.size() == 0) {
      return false;
    } else {
      bool res = true;
      for (Type* typ : subtypes) {
        res = res && typ->is_complete();
      }
      return res;
    }
  }
  return true;
}
