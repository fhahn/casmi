#include <assert.h>

#include <string>
#include <map>
#include <sstream>

#include "macros.h"

#include "libsyntax/types.h"


Type::Type(const std::string& type_name, std::vector<Type*>& internal_types) : unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
  if (type_name == "List") {
    t = TypeType::LIST;

    if (internal_types.size() == 0) {
      internal_type = new Type(TypeType::UNKNOWN);
    } else if (internal_types.size() == 1) {
      internal_type = internal_types[0];
    } else {
      t = TypeType::INVALID;
      internal_type = nullptr;
    }
  } else if (type_name == "Tuple") {
    if (internal_types.size() > 0) {
      t = TypeType::TUPLE;
      tuple_types = internal_types;
    } else {
    DEBUG("INVALID");
      t = TypeType::INVALID;
    }
  }

}

Type::Type(TypeType typ, std::vector<Type*>& internal_types) : t(typ), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
  if (t == TypeType::LIST) {
    if (internal_types.size() == 0) {
      internal_type = new Type(TypeType::UNKNOWN);
    } else if (internal_types.size() == 1) {
      internal_type = internal_types[0];
    } else {
      t = TypeType::INVALID;
      internal_type = nullptr;
    }

  } else if (t == TypeType::TUPLE_OR_LIST) {
    tuple_types = internal_types;
  } else {
    DEBUG("INVALID");
    t = TypeType::INVALID;
  }
}

Type::Type(TypeType typ, Type *int_typ) : t(typ), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
  if (t != TypeType::LIST && t != TypeType::TUPLE_OR_LIST) {
    t = TypeType::INVALID;
  }
  internal_type = int_typ;
}

Type::Type() : t(TypeType::INVALID), internal_type(nullptr), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {}

Type::Type(TypeType t) : t(t), internal_type(nullptr), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
  if (t == TypeType::LIST || t == TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
}

Type::Type(Type *other) : t(other->t), internal_type(nullptr), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
  if (other->internal_type != nullptr) {
    internal_type = new Type(other->internal_type);
  } else {
    internal_type = nullptr;
  }
}


Type::Type(const Type& other) : t(other.t), internal_type(other.internal_type), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types()  {
}

Type::Type(const std::string& type_name) : internal_type(nullptr), unify_with_left(nullptr), unify_with_right(nullptr), constraints(), tuple_types() {
  if (type_name == "Int") { t = TypeType::INT; }
  else if (type_name == "Float") { t = TypeType::FLOAT; }
  else if (type_name == "Undef") { t = TypeType::UNDEF; }
  else if (type_name == "Boolean") { t = TypeType::BOOLEAN; }
  else if (type_name == "RuleRef") { t = TypeType::RULEREF; }
  else if (type_name == "String") { t = TypeType::STRING; }
  else { t = TypeType::INVALID; }
}

bool Type::eq(const Type& other) const {
  if (t != other.t) {
    return false;
  }

  if (t == TypeType::LIST) {
    return *internal_type == *other.internal_type;
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
    case TypeType::UNDEF: return "Undef";
    case TypeType::BOOLEAN: return "Boolean";
    case TypeType::RULEREF: return "RuleRef";
    case TypeType::STRING: return "String";
    case TypeType::LIST: return "List("+internal_type->to_str()+")";
    case TypeType::UNKNOWN: return "Unknown";
    case TypeType::SELF: return "Self";
    case TypeType::TUPLE_OR_LIST: /*{
      std::string res = "TupleOrList (";
      for (Type* t:tuple_types) {
        res += t->to_str() + ",";
      }
      return res + ")";
    } */
    case TypeType::TUPLE:{
      std::string res = "Tuple (";
      for (Type* t:tuple_types) {
        res += t->to_str() + ",";
      }
      return res + ")";
    }
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


bool Type::unify_nofollow(Type *other) {
  bool result = true;
  if (t != TypeType::UNKNOWN && other->t != TypeType::UNKNOWN) {
    if (t != TypeType::LIST && t != TypeType::TUPLE && t != TypeType::TUPLE_OR_LIST) {
      return t == other->t;
    } else if(other->t == TypeType::LIST && t == TypeType::LIST) {
      result = internal_type->unify(other->internal_type);
    } else if(other->t == TypeType::LIST && t == TypeType::TUPLE_OR_LIST) {
      bool internal_complete = other->internal_type->is_complete();
      for (Type* typ : tuple_types) {
        result = result && typ->unify(other->internal_type);
      }
      if (result) {
        t = TypeType::LIST;
        internal_type = new Type(other->internal_type);
        tuple_types.clear();
      } else {
        if (!internal_complete) {
          other->internal_type->t = TypeType::UNKNOWN;
        }
      }
      return result;
    } else if(other->t == TypeType::TUPLE_OR_LIST && t == TypeType::LIST) {
      bool internal_complete = internal_type->is_complete();
      for (Type* typ : other->tuple_types) {
        result = result && typ->unify(internal_type);
      }
      if (result) {
        other->t = TypeType::LIST;
        other->internal_type = new Type(internal_type);
        other->tuple_types.clear();
      } else {
        if (!internal_complete) {
          internal_type->t = TypeType::UNKNOWN;
        }
      }
      return result;
    } else if(t == TypeType::TUPLE && other->t == TypeType::LIST) {
      if (!other->is_complete()) {
        return false;
      }
      for (Type* typ : tuple_types) {
        result = result && typ->unify(other->internal_type);
      }
      if (result) {
        other->t = TypeType::TUPLE;
        other->tuple_types = tuple_types;
      }
      return result;
    } else if(other->t == TypeType::TUPLE && t == TypeType::LIST) {
      if (!is_complete()) {
        return false;
      }
      for (Type* typ : other->tuple_types) {
        result = result && typ->unify(internal_type);
      }
      if (result) {
        t = TypeType::TUPLE;
        tuple_types = other->tuple_types;
      }
      return result;
    } else if (other->t == TypeType::TUPLE && (t == TypeType::TUPLE_OR_LIST || t == TypeType::TUPLE)) {
      t = TypeType::TUPLE;
      if (tuple_types.size() == 0) {
        tuple_types = other->tuple_types;
        return true;
      } else if (other->tuple_types.size() == 0) {
        other->tuple_types = tuple_types;
        return true;
      } else if (tuple_types.size() == other->tuple_types.size()) {
        for (size_t i=0; i<tuple_types.size(); i++) {
          result = result && tuple_types[i]->unify(other->tuple_types[i]);
        }
        return result;
      } else {
        return false;
      }
    } else if (t == TypeType::TUPLE && (other->t == TypeType::TUPLE_OR_LIST || other->t == TypeType::TUPLE)) {
      other->t = TypeType::TUPLE;
      if (tuple_types.size() == 0) {
        tuple_types = other->tuple_types;
        return true;
      } else if (other->tuple_types.size() == 0) {
        other->tuple_types = tuple_types;
        return true;
      } else if (tuple_types.size() == other->tuple_types.size()) {
        for (size_t i=0; i<tuple_types.size(); i++) {
          result = result && tuple_types[i]->unify(other->tuple_types[i]);
        }
        return result;
      } else {
        return false;
      }
    } else if (t == TypeType::TUPLE_OR_LIST && other->t == TypeType::TUPLE_OR_LIST) {
      if (tuple_types.size() == 0) {
        tuple_types = other->tuple_types;
        return true;
      } else if (other->tuple_types.size() == 0) {
        other->tuple_types = tuple_types;
        return true;
      } else if (tuple_types.size() == other->tuple_types.size()) {
        for (size_t i=0; i<tuple_types.size(); i++) {
          result = result && tuple_types[i]->unify(other->tuple_types[i]);
        }
        return result;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  if (t == TypeType::UNKNOWN && (other->t == TypeType::TUPLE || other->t == TypeType::TUPLE_OR_LIST)) {
    t = other->t;
    tuple_types = other->tuple_types;
    return true;
  }

  if (t == TypeType::UNKNOWN && other->t == TypeType::LIST) {
    internal_type = new Type(TypeType::UNKNOWN);
    t = TypeType::LIST;
    return other->internal_type->unify(internal_type);
  }

  if (t == TypeType::UNKNOWN && other->t != TypeType::UNKNOWN) {
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

  if ((t == TypeType::TUPLE || t == TypeType::TUPLE_OR_LIST) && other->t == TypeType::UNKNOWN) {
    other->t = t;
    other->tuple_types = tuple_types;
    return true;
  }

  if (t == TypeType::LIST && other->t == TypeType::UNKNOWN) {
    other->internal_type = new Type(TypeType::UNKNOWN);
    other->t = TypeType::LIST;
    return internal_type->unify(other->internal_type);
  }

  if (t != TypeType::UNKNOWN && other->t == TypeType::UNKNOWN) {
    if (t != TypeType::LIST) {
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
    } else {
      other->internal_type = new Type(internal_type);
      t = TypeType::LIST;
      return true;
    }
  }

  if (other->t != TypeType::UNKNOWN && t == TypeType::UNKNOWN) {
    if (other->t != TypeType::LIST) {
      t = other->t;
      bool matched_constraint = true;
      for (Type constraint : constraints) {
        if (other->unify(constraint)) {
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
    } else {
      internal_type = new Type(other->internal_type);
      other->t = TypeType::LIST;
      return true;
    }
  }

  return result;
}

bool Type::unify_left(Type *other) {
  DEBUG("LEFT "<<other);
  if (other == nullptr) {
    return true;
  }

  
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
  } else if ((t == TypeType::TUPLE || t == TypeType::TUPLE_OR_LIST) && tuple_types.size() == 0) {
    return true;
  } 
  return false;
}

bool Type::unify(Type *other) {
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
  if (internal_type) {
    return internal_type->is_complete();
  }
  if (t == TypeType::TUPLE || t == TypeType::TUPLE_OR_LIST) {
    if (tuple_types.size() == 0) {
      return false;
    } else {
      bool res = true;
      for (Type* typ : tuple_types) {
        res = res && typ->is_complete();
      }
      return res;
    }
  }
  return true;
}

FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
