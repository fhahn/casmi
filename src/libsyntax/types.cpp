#include <assert.h>

#include <string>
#include <map>

#include "macros.h"

#include "libsyntax/types.h"

Type::Type(const std::string& type_name, std::vector<Type>& internal_types) : unify_with(nullptr), constraints()  {
  if (type_name == "List") {
    t = TypeType::LIST;
  } else {
    t = TypeType::INVALID;
  }

  if (internal_types.size() == 0) {
    internal_type = new Type(TypeType::UNKNOWN);
  } else if (internal_types.size() == 1) {
    internal_type = new Type(&internal_types[0]);
  } else {
    t = TypeType::INVALID;
    internal_type = nullptr;
  }
}

Type::Type(TypeType typ, std::vector<Type>& internal_types) : t(typ), unify_with(nullptr), constraints() {
  if (t != TypeType::LIST && t != TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
  if (internal_types.size() == 0) {
    internal_type = new Type(TypeType::UNKNOWN);
  } else if (internal_types.size() == 1) {
    internal_type = new Type(&internal_types[0]);
  } else {
    t = TypeType::INVALID;
    internal_type = nullptr;
  }
}

Type::Type(TypeType typ, Type& int_typ) : t(typ), unify_with(nullptr), constraints() {
  if (t != TypeType::LIST && t != TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
  internal_type = new Type(int_typ);
}

Type::Type() : t(TypeType::INVALID), internal_type(nullptr), unify_with(nullptr), constraints() {}

Type::Type(TypeType t) : t(t), internal_type(nullptr), unify_with(nullptr), constraints() {
  if (t == TypeType::LIST || t == TypeType::TUPLE) {
    t = TypeType::INVALID;
  }
}

Type::Type(Type *other) : t(other->t), internal_type(nullptr), unify_with(nullptr), constraints() {
  if (other->internal_type != nullptr) {
    internal_type = new Type(other->internal_type);
  } else {
    internal_type = nullptr;
  }
}


Type::Type(const Type& other) : t(other.t), internal_type(other.internal_type), unify_with(nullptr), constraints() {
}

Type::Type(const std::string& type_name) : internal_type(nullptr), unify_with(nullptr) {
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
    case TypeType::UNKNOWN: return "Unkown";
    case TypeType::SELF: return "Self";
    case TypeType::INVALID: return "Invalid";
    default: assert(0);
  }
}

std::string Type::unify_links_to_str() const {
  std::string res = std::to_string((unsigned long) this);
  DEBUG("-> "<<this);
  if (unify_with) {
    unify_with->unify_links_to_str();
  }
  return res;
}

bool Type::unify(Type other) {
  if (t == TypeType::UNKNOWN) {
    t = other.t;
    return true;
  }
  return t == other.t;
}




bool Type::unify(Type *other) {
  DEBUG("IN UNIFY "<< this << " "<<other);
  bool result = false;
  if (t != TypeType::UNKNOWN && other->t != TypeType::UNKNOWN) {
    if (t != TypeType::LIST) {
      result = t == other->t;
    } else {
      if (other->t == TypeType::LIST) {
        result = internal_type->unify(other->internal_type);
        if (unify_with != nullptr && unify_with != other) {
          DEBUG("UNIFIED1 "<<this<< " and "<< unify_with <<" TO "<<to_str());
          result = unify(unify_with);
        }
        if (other->unify_with != nullptr && unify_with != other) {
          DEBUG("UNIFIED2 "<<this<< " and "<< other->unify_with <<" TO "<<to_str());
          result = unify(other->unify_with);
        }
        return result;
      } else {
        return false;
      }
    }
  }

  if (t == TypeType::UNKNOWN && other->t == TypeType::LIST) {
    internal_type = new Type(other->internal_type);
    t = TypeType::LIST;

    if (internal_type->t == TypeType::UNKNOWN) {
      Type* last_link = this;

      while (last_link->unify_with != nullptr) {
        last_link = last_link->unify_with;
      }

      DEBUG("UNIFY link "<<other<<" to unify with "<<last_link);
      last_link->unify_with = other;
      assert(last_link != other);
    }
    return  true;
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
    if (unify_with != nullptr) {
      DEBUG("UNIFIED "<<this<< " and "<< other <<" TO "<<to_str());
      result = other->unify(unify_with);
    }

    DEBUG("UNIFIED "<<this<< " and "<< other <<" TO "<<to_str());
    return true;
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

      if (other->unify_with != nullptr) {
        result = unify(other->unify_with);
      }

      DEBUG("UNIFIED "<<this<< " and "<< other <<" TO "<<to_str());
      result = true;
    } else {
      other->internal_type = new Type(internal_type);
      t = TypeType::LIST;
      result = true;
    }
  }


  if (t == TypeType::UNKNOWN && other->t == TypeType::UNKNOWN) {
    Type* last_link = this;

    while (last_link->unify_with != nullptr) {
      last_link = last_link->unify_with;
    }

    DEBUG("UNIFY link "<<other<<" to unify with "<<this);
    last_link->unify_with = other;
    result = true;
  }
  //DEBUG("UNIFY: " <<result);
  return result;
}


bool Type::is_complete() {
  if (t == TypeType::UNKNOWN) {
    return false;
  }
  if (internal_type) {
    return internal_type->is_complete();
  }
  return true;
}

FunctionInfo::FunctionInfo(std::vector<Type> *args, Type return_type) :
    arguments_(args), return_type_(return_type) {}
