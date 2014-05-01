#include <hayai.hpp>

#include <cassert>
#include <vector>

#include "macros.h"
#include "libsyntax/types.h"


#define NUM_ITERATIONS 100


class TestClass {
  public:
    size_t a;
    size_t b;
    size_t c;
    size_t d;
    size_t e;
    size_t f;
    size_t g;
    size_t h;
    size_t i;
    size_t j;

    TestClass(size_t a, size_t b, size_t c, size_t d, size_t e, size_t f, size_t g, size_t h, size_t i, size_t j) : a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h), i(i), j(j) {}
};

class Tester {
  public:
    virtual std::vector<size_t>* add_and_return_pointer(std::vector<size_t>* v, size_t val) {
      v->push_back(val+10);
      return v;
    }

    virtual std::vector<size_t> add_and_return_move(std::vector<size_t> v, size_t val) {
      v.push_back(val+10);
      return std::move(v);
    }

    virtual std::vector<size_t> add_and_return_copy(std::vector<size_t> v, size_t val) {
      v.push_back(val+10);
      return (v);
    }

    virtual TestClass* add_and_return_pointer(TestClass *o, size_t val) {
      o->a = val;
      return o;
    }

    virtual TestClass add_and_return_move(TestClass o, size_t val) {
      o.a = val;
      return std::move(o);
    }
};

BENCHMARK(PointerVsMove, vector_with_move, 10, NUM_ITERATIONS) {
  Tester t;
  std::vector<size_t> vec;
  for (size_t i=0; i < 100000; i++) {
    vec = t.add_and_return_move(std::move(vec), i);
  }
}



BENCHMARK(PointerVsMove, vector_with_pointer, 10, NUM_ITERATIONS) {
  Tester t;
  std::vector<size_t> *vec = new std::vector<size_t>();
  for (size_t i=0; i < 100000; i++) {
    vec = t.add_and_return_pointer(vec, i);
  }
}

static size_t MAX = 10000000000;
BENCHMARK(PointerVsMove, class_with_pod_pointer, 10, NUM_ITERATIONS) {
  Tester t;
  TestClass *o = new TestClass(1,2,3,4,5,6,7,8,9,10);
  for (size_t i=0; i < MAX; i++) {
    o = t.add_and_return_pointer(o, i);
  }
}

BENCHMARK(PointerVsMove, class_with_pod_move, 10, NUM_ITERATIONS) {
  Tester t;
  TestClass o(1,2,3,4,5,6,7,8,9,10);
  for (size_t i=0; i < MAX; i++) {
    o = t.add_and_return_move(std::move(o), i);
  }
}


/*

BENCHMARK(PointerVsMove, vector_with_copy, 10, NUM_ITERATIONS) {
  Tester t;
  std::vector<size_t> vec;
  for (size_t i=0; i < 100000; i++) {
    vec = t.add_and_return_move(vec, i);
  }
} */
