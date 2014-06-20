#include <cassert>
#include <sstream>

#include "libinterpreter/symbolic.h"
#include "libinterpreter/operators.h"

namespace symbolic {
  static uint32_t last_symbol_id = 1;

  uint32_t next_symbol_id() {
    last_symbol_id += 1;
    return last_symbol_id;
  }

  static uint32_t current_time = 2;
  void advance_timestamp() {
    current_time += 1;
  }

  uint32_t get_timestamp() {
    return current_time;
  }

  std::string arguments_to_string(const Function *func, const uint64_t args[], 
                                  uint16_t sym_args, bool strip=false) {
    std::stringstream ss;
    ss << ',';

    for (uint32_t i = 0; i < func->arguments_.size(); i++) {
      if ((sym_args & (1 << i)) != 0) {
        ss << "sym" << (INT_T) args[i];
      } else {
        switch (func->arguments_[i]->t) {
          case TypeType::INT:
            ss << (INT_T) args[i];
            break;
          default: assert(0);
        }
      }
      ss << ',';
    }
    // Strip leading and trailing comma if requested
    if (strip) {
      return ss.str().substr(1, ss.str().size()-2);
    } else {
      return ss.str();
    }
  }


  std::string location_to_string(const Function *func, const uint64_t args[],
                                 uint16_t sym_args, const value_t& val, uint32_t time) {
    std::stringstream ss;
    if (func->is_static) {
      ss << "cs";
    } else {
      ss << "st";
    }

    ss << func->name << "(" << time
       << arguments_to_string(func, args, sym_args) << val.to_str(true)
       << ")";
    return ss.str();
  }


  static void dump_type(std::stringstream& ss, const value_t& v) {
    if (v.is_symbolic() && !v.value.sym->type_dumped) {
      ss << "tff(symbolNext, type, " << v.to_str() << ": $int)."
         << std::endl;
      v.value.sym->type_dumped = true;
    }
  }

  void dump_create(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const value_t& v) {
    std::stringstream ss;
    dump_type(ss, v);
    ss << "fof(id%u,hypothesis,"
       << location_to_string(func, args, sym_args, v, symbolic::get_timestamp()-1)
       << ").%%CREATE: " << func->name;

    if (func->arguments_.size() == 0) {
       ss << arguments_to_string(func, args, sym_args, true);
    } else {
       ss << '(' << arguments_to_string(func, args, sym_args, true) << ')';
    }

    ss << std::endl;
    trace.push_back(ss.str());

    if (symbolic::get_timestamp() > 2) {
      for (uint32_t i=1; i < symbolic::get_timestamp()-1; i++) {
        std::stringstream ss;
        ss << "fof(id%u,hypothesis,"
           << location_to_string(func, args, sym_args, v, i)
           << ").%%CATCHUP: " << func->name;

        if (func->arguments_.size() == 0) {
           ss << arguments_to_string(func, args, sym_args, true);
        } else {
           ss << '(' << arguments_to_string(func, args, sym_args, true) << ')';
        }

        ss << std::endl;
        trace.push_back(ss.str());
      }
    }
  }

  void dump_symbolic(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const value_t& v) {
     std::stringstream ss;
    ss << "fof(id%u,hypothesis,"
       << location_to_string(func, args, sym_args, v, get_timestamp())
       << ").%%SYMBOLIC: " << func->name;

    if (func->arguments_.size() == 0) {
       ss << arguments_to_string(func, args, sym_args, true);
    } else {
       ss << '(' << arguments_to_string(func, args, sym_args, true) << ')';
    }
    ss << std::endl;
    trace.push_back(ss.str());
  }

  void dump_update(std::vector<std::string>& trace, const Function *func,
      const uint64_t args[], uint16_t sym_args, const value_t& v) {
    std::stringstream ss;
    dump_type(ss, v);
    ss << "fof(id%u,hypothesis,"
       << location_to_string(func, args, sym_args, v, get_timestamp())
       << ").%%UPDATE: " << func->name;

    if (func->arguments_.size() == 0) {
       ss << arguments_to_string(func, args, sym_args, true);
    } else {
       ss << '(' << arguments_to_string(func, args, sym_args, true) << ')';
    }
    ss << std::endl;
    trace.push_back(ss.str());
  }

  void dump_pathcond_match(std::vector<std::string>& trace, const std::string &filename,
      size_t lineno, const symbolic_condition *cond, bool status) {
    std::stringstream ss;
    ss << "% " << filename << ":" << lineno << " PC-LOOKUP ("
       << cond->to_str() << ") = " << status << std::endl;
    trace.push_back(ss.str());
 
  }

  void dump_if(std::vector<std::string>& trace, const std::string &filename,
      size_t lineno, const symbolic_condition *cond) {
    std::stringstream ss;
    ss << "fof('id" << filename <<  ":" << lineno << "',hypothesis,"
       << cond->to_str() << ")." << std::endl;
    trace.push_back(ss.str());
  }

  void dump_final(std::vector<std::string>& trace, const std::vector<std::pair<const Function*,
      std::unordered_map<ArgumentsKey, value_t> >>& functions) {
    std::stringstream ss;
    uint32_t i = 0;
    for (auto& pair : functions) {
      if (!pair.first->is_symbolic) {
        continue;
      }
      for (auto& value_pair : pair.second) {
        ss << "fof(final" << i << ",hypothesis,"
           << location_to_string(pair.first, value_pair.first.p, value_pair.first.sym_args, value_pair.second, 0)
           << ").%FINAL: " << pair.first->name;
        if (pair.first->arguments_.size() == 0) {
          ss << arguments_to_string(pair.first, value_pair.first.p, value_pair.first.sym_args, true);
        } else {
          ss << '(' << arguments_to_string(pair.first, value_pair.first.p, value_pair.first.sym_args, true) << ')' ;
        }
        ss << std::endl;
        i += 1;
      }
    }
    trace.push_back(ss.str());
  }

  check_status_t check_inclusion(const symbolic_condition& known, const symbolic_condition& check) {
    switch (check.op) {
      case ExpressionOperation::EQ:
        if (known.op == ExpressionOperation::EQ) {
          if (*known.rhs == *check.rhs) {
            return check_status_t::TRUE;
          } else {
            return check_status_t::FALSE;
          }
        } else if (known.op == ExpressionOperation::NEQ) {
          if(*known.rhs == *check.rhs) {
            return check_status_t::FALSE;
          } 
        }
        return check_status_t::NOT_FOUND;
      case ExpressionOperation::NEQ:
        if (known.op == ExpressionOperation::NEQ) {
          if(*known.rhs == *check.rhs) {
            return check_status_t::TRUE;
          }
        } else if (known.op == ExpressionOperation::EQ) {
          if (*known.rhs == *check.rhs) {
            return check_status_t::FALSE;
          } else {
            return check_status_t::TRUE;
          }
        }
        return check_status_t::NOT_FOUND;
      case ExpressionOperation::LESSEREQ:
        if (known.op == ExpressionOperation::EQ) {
          value_t res = operators::lessereq(*known.rhs, *check.rhs);
          if (res.value.boolean) {
            return check_status_t::TRUE;
          } else {
            return check_status_t::FALSE;
          }
        } else if (known.op == ExpressionOperation::LESSEREQ) {
          value_t res = operators::lessereq(*check.rhs, *known.rhs);
          if (res.value.boolean) {
            return check_status_t::TRUE;
          }
        } else if (known.op == ExpressionOperation::GREATER) {
          value_t res = operators::lessereq(*check.rhs, *known.rhs);
          if (res.value.boolean) {
            return check_status_t::FALSE;
          }
        }
        return check_status_t::NOT_FOUND;
      case ExpressionOperation::GREATER:
        if (known.op == ExpressionOperation::EQ) {
          value_t res = operators::greater(*known.rhs, *check.rhs);
          if (res.value.boolean) {
            return check_status_t::TRUE;
          } else {
            return check_status_t::FALSE;
          }
        } else if (known.op == ExpressionOperation::LESSEREQ) {
          value_t res = operators::lessereq(*known.rhs, *check.rhs);
          if (res.value.boolean) {
            return check_status_t::FALSE;
          }
        } else if (known.op == ExpressionOperation::GREATER) {
          value_t res = operators::greater(*check.rhs, *known.rhs);
          if (res.value.boolean) {
            return check_status_t::TRUE;
          }
        }
        return check_status_t::NOT_FOUND;

      default:
        assert(0);
    }
    return check_status_t::NOT_FOUND;
  }

  check_status_t check_condition(std::vector<symbolic_condition*> known_conditions,
      const symbolic_condition *check) {

    symbolic_condition cond(check->lhs, check->rhs, check->op);

    if (check->lhs->type != TypeType::SYMBOL) {
      if (check->rhs->type == TypeType::SYMBOL) {
        cond = symbolic_condition(check->rhs, check->lhs, check->op);
      } else {
        throw RuntimeException("Invalid condition passed");
      }
    }

    for (symbolic_condition *known_cond : known_conditions) {
     check_status_t s = check_status_t::NOT_FOUND;
      if (*(known_cond->lhs) == *(cond.lhs)) {
        s = check_inclusion(*known_cond, cond);
      } else if (*(known_cond->rhs) == *(cond.lhs)) {
        s = check_inclusion(
            symbolic_condition(known_cond->rhs, known_cond->lhs, known_cond->op),
            cond);
      }
      if (s != check_status_t::NOT_FOUND) {
        return s;
      }
    }
    return check_status_t::NOT_FOUND;
  }

  uint32_t dump_listconst(std::vector<std::string>& trace, List *l) {
    auto iter = l->begin();
    auto end = l->end();
    uint32_t sym_id = 0;
    if (iter != end) {
      sym_id = symbolic::next_symbol_id();
      std::stringstream ss;
      ss << "tff(symbolNext, type, sym" << sym_id << ": $int)."
         << std::endl;
      ss << "fof(id%u,hypothesis,fcons(eEmptyList," << (*iter).to_str(true)
         << ",sym" << sym_id << "))." << std::endl;
      iter++;
      trace.push_back(ss.str());

      for (;iter != end; iter++) {
        std::stringstream ss;
        uint32_t next_id = symbolic::next_symbol_id();
        ss << "tff(symbolNext, type, sym" << next_id << ": $int)."
           << std::endl;
        ss << "fof(id%u,hypothesis,fcons(sym" << sym_id << "," << (*iter).to_str(true)
           << ",sym" << next_id << "))." << std::endl;
        sym_id = next_id;
        trace.push_back(ss.str());
      }
    }
    return sym_id;
  }

  void dump_builtin(std::vector<std::string>& trace, const char *name,
                    const std::vector<value_t>& args, const value_t& ret) {
    std::stringstream ss;

    for (const auto& a : args) {
      dump_type(ss, a);
    }
    dump_type(ss, ret);

    ss << "fof(id%u,hypothesis,f" << name << "(";

    for (const auto& a : args) {
      ss << a.to_str() << ", ";
    }
    ss << ret.to_str() << "))." << std::endl;
    trace.push_back(ss.str());
  }
}
