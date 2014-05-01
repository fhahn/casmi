#ifndef CASMI_LIBINTERPRETER_VALUE_H
#define CASMI_LIBINTERPRETER_VALUE_H


#include "libsyntax/types.h"
#include "libutil/exceptions.h"

#include "librt/rt.h"

class RuleNode;

class HeadList;
class BottomList;
class List;

class Value {
  public:
    TypeType type;
    union {
      INT_T ival;
      FLOAT_T fval;
      bool bval;
      RuleNode *rule;
      std::string *string;
      List *list;
    } value;

    Value();
    Value(INT_T ival);
    Value(FLOAT_T fval);
    Value(bool bval);
    Value(RuleNode *rule);
    Value(std::string *string);
    Value(const Type& t, List *list);
    Value(Value& other);
    Value(const Value& other);
    Value(Value&& other);
    Value(const Type& type, casm_update* update);

    Value& operator=(const Value& other);

    void add(const Value& other);
    void sub(const Value& other);
    void mul(const Value& other);
    void div(const Value& other);
    void mod(const Value& other);
    void rat_div(const Value& other);

    void eq(const Value& other);

    void lesser(const Value& other);
    void greater(const Value& other);
    void lessereq(const Value& other);
    void greatereq(const Value& other);

    uint64_t to_uint64_t() const;

    bool is_undef() const;

    std::string to_str() const;
};


bool value_eq(const Value& v1, const Value& v2);


class List {

  protected:
    enum class ListType {
      HEAD,
      BOTTOM,
      SKIP,
    };

  public:
    
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

class BottomList : public List {
  friend class List;

  private:
    uint32_t usage_count;
    bool allocated_in_collect; 

  public:
    std::vector<Value> values;

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

namespace std {

  template <> struct hash<HeadList>;

  template <> struct hash<Value> {
    static std::hash<std::string> str_hasher;
    static std::hash<HeadList> head_list_hasher;
    static std::hash<BottomList> perm_list_hasher;

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
