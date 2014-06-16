#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H


#include "macros.h"

#include "libsyntax/types.h"
#include "libsyntax/symbols.h"
#include "libutil/exceptions.h"

#include "libcasmrt/rt.h"


struct enum_value_t;

class RuleNode;
enum class ExpressionOperation;

class HeadList;
class TailList;
class BottomList;
class List;

struct symbolic_condition;
struct symbol_t;
struct rational_t;


class Value {
  public:
    TypeType type;
    union {
      INT_T integer;
      FLOAT_T float_;
      bool boolean;
      RuleNode *rule;
      std::string *string;
      List *list;
      const enum_value_t *enum_val;
      const rational_t *rat;
      symbol_t *sym;
    } value;

    Value();
    Value(INT_T integer);
    Value(FLOAT_T float_);
    Value(bool boolean);
    Value(RuleNode *rule);
    Value(std::string *string);
    Value(const Type& t, List *list);
    Value(const enum_value_t *enum_val);
    Value(const rational_t *val);
    Value(symbol_t *sym);
    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value(TypeType type, casm_update* update);

    Value& operator=(const Value& other);
    bool operator==(const Value &other) const;
    bool operator!=(const Value &other) const;

    uint64_t to_uint64_t() const;

    bool is_undef() const;
    bool is_symbolic() const;

    const std::string to_str(bool symbolic=false) const;
};

struct symbol_t {
  const uint32_t id;
  symbolic_condition *condition;
  bool type_dumped;
  bool update_dumped;
  List *list; // used for symbolic lists
  // The distinction between concrete lists and symbolic lists can be fuzzy,
  // because fcons formulas are generated for all list constants by the legacy
  // interpreter

  symbol_t(uint32_t id);
  symbol_t(uint32_t id, symbolic_condition *cond);
};

struct symbolic_condition {
  Value *lhs;
  Value *rhs;
  ExpressionOperation op;

  symbolic_condition(Value *lhs, Value *rhs, ExpressionOperation op);
  std::string to_str() const;
};

struct rational_t {
  int64_t numerator;
  int64_t denominator;

  rational_t();
  rational_t(int64_t num, int64_t denom);
  rational_t(const rational_t& other);

  bool operator==(const rational_t& other) const;
  const rational_t& operator+(const rational_t& other) const;
  const rational_t& operator-(const rational_t& other) const;
  const rational_t& operator*(const rational_t& other) const;
  const rational_t& operator/(const rational_t& other) const;
  const rational_t& operator%(const rational_t& other) const;

  const std::string to_str() const;
};

class List {
  public:
    enum class ListType {
      HEAD,
      BOTTOM,
      SKIP,
      TAIL,
    };
   
    class const_iterator {
      public:
        typedef const_iterator self_type;
        typedef Value value_type;
        typedef Value& reference;
        typedef int difference_type;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(const List *ptr);
        const_iterator(const self_type& other);
        self_type operator++();
        self_type operator++(int);
        void next();
        const Value& operator*();
        //const pointer operator->() { return ptr_; }
        bool operator==(const self_type& rhs) const;
        bool operator!=(const self_type& rhs) const;

      private:
        const BottomList *bottom;
        const HeadList *head;
        const TailList *tail;
        uint32_t pos;

        void do_init(const List *other);
    };

    ListType list_type;

    List(ListType t);
    virtual ~List() {}


    bool operator==(const List& other) const;
    bool operator!=(const List& other) const;

    bool is_bottom() const;
    bool is_head() const;
    bool is_skip() const;
    bool is_tail() const;

    const_iterator begin() const;
    const_iterator end() const;

    const std::string to_str() const;

    void bump_usage();
    void decrease_usage();

    BottomList* collect();
};


class HeadList : public List {
  public:
    List* right;
    const Value current_head;

    HeadList(List* right, const Value& val);
};

class TailList : public List {
  public:
    TailList* right;
    const Value current_tail;

    TailList(TailList* right, const Value& val);

    void collect(std::vector<Value>& collect_to);
};


class BottomList : public List {
  friend class List;

  private:
    uint32_t usage_count;
    bool allocated_in_collect; 

  public:
    std::vector<Value> values;

    TailList *tail;

    BottomList();
    BottomList(const std::vector<Value>& vals);

    virtual ~BottomList();

    bool is_used() const;
    bool check_allocated_and_set_to_false();
};

class SkipList : public List {
  public:
    size_t skip;
    BottomList* bottom;

    SkipList(size_t skip, BottomList *btm);
};

size_t hash_uint64_value(const Type *type, uint64_t val);
bool eq_uint64_value(const Type *type, uint64_t lhs, uint64_t rhs);

namespace std {

  template <> struct hash<HeadList>;

  template <> struct hash<Value> {
    size_t operator()(const Value &key) const;
  };

  template <> struct hash<std::vector<Value>> {
    static std::hash<Value> hasher;

    size_t operator()(const std::vector<Value> &key) const;
  };

  template <> struct hash<HeadList> {
    static std::hash<Value> hasher;

    size_t operator()(const HeadList &key) const;
  };

  template <> struct hash<BottomList> {
    static std::hash<std::vector<Value>> list_hasher;

    size_t operator()(const BottomList &key) const;
  };
}


#endif
