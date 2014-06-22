// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <iostream>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libinterpreter/value.h"


class ListIteratorTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }
};

TEST_F(ListIteratorTest, test_iterator_bottom_list) {
  BottomList lst;
  lst.values = { value_t((INT_T)1), value_t((INT_T)2), value_t((INT_T)3) };

  auto iter = lst.begin();
  EXPECT_EQ(3, (*iter).value.integer);

  iter++;
  EXPECT_EQ(2, (*iter).value.integer);

  iter++;
  EXPECT_EQ(1, (*iter).value.integer);

  iter++;
  EXPECT_EQ(lst.end(), iter);
}

TEST_F(ListIteratorTest, test_iterator_eq_begin_end) {
  BottomList list;
  list.values = { value_t((INT_T)1), value_t((INT_T)2), value_t((INT_T)3) };

  EXPECT_TRUE(list.begin() != list.end());
}

TEST_F(ListIteratorTest, test_iterator_head_and_tail_list) {
  BottomList list;
  list.values = { value_t((INT_T)1), value_t((INT_T)2), value_t((INT_T)3) };

  TailList tail(nullptr, value_t((INT_T)5));
  list.tail = &tail;

  HeadList head(&list, value_t((INT_T)4));

  auto iter = head.begin();
  EXPECT_EQ(4, (*iter).value.integer);

  iter++;
  EXPECT_EQ(3, (*iter).value.integer);

  iter++;
  EXPECT_EQ(2, (*iter).value.integer);

  iter++;
  EXPECT_EQ(1, (*iter).value.integer);

  iter++;
  EXPECT_EQ(5, (*iter).value.integer);

  iter++;
  EXPECT_EQ(list.end(), iter);
}


class ListTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }
};

TEST_F(ListTest, test_list_eq_sublists) {
  BottomList list;
  list.values = { value_t((INT_T)1), value_t((INT_T)2), value_t((INT_T)3) };

  TailList tail(nullptr, value_t((INT_T)5));
  list.tail = &tail;

  HeadList head(&list, value_t((INT_T)4));

  EXPECT_EQ(true, head == head);
  EXPECT_EQ(true, list == list);
  EXPECT_EQ(true, tail == tail);

  EXPECT_EQ(true, tail != head);
  EXPECT_EQ(true, head != list);
  EXPECT_EQ(true, list != tail);
}

TEST_F(ListTest, test_list_eq_head_lists) {
  HeadList h11(nullptr, value_t((INT_T)3));
  HeadList h12(&h11, value_t((INT_T)2));
  HeadList h13(&h12, value_t((INT_T)1));

  HeadList h21(nullptr, value_t((INT_T)3));
  HeadList h22(&h21, value_t((INT_T)2));
  HeadList h23(&h22, value_t((INT_T)1));

  EXPECT_TRUE(h11 == h21);
  EXPECT_TRUE(h11 != h22);
  EXPECT_TRUE(h12 == h22);
  EXPECT_TRUE(h12 != h21);
  EXPECT_TRUE(h13 == h23);
}

TEST_F(ListTest, test_list_eq_head_bottom_lists) {
  HeadList h1(nullptr, value_t((INT_T)3));
  HeadList h2(&h1, value_t((INT_T)2));
  HeadList h3(&h2, value_t((INT_T)1));

  BottomList bottom3;
  bottom3.values = { value_t((INT_T)3), value_t((INT_T)2), value_t((INT_T)1) };

  BottomList bottom2;
  bottom2.values = { value_t((INT_T)3), value_t((INT_T)2) };

  BottomList bottom1;
  bottom1.values = { value_t((INT_T)3) };

  EXPECT_TRUE(bottom3 == h3);
  EXPECT_TRUE(bottom2 == h2);
  EXPECT_TRUE(bottom1 == h1);

  EXPECT_TRUE(bottom2 != h3);
  EXPECT_TRUE(bottom2 != h1);
  EXPECT_TRUE(bottom1 != h2);

}
