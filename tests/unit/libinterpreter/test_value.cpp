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

TEST_F(ListIteratorTest, test_iterator_perm_list) {
  PermList perm;
  perm.values = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  auto iter = perm.begin();
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);

  iter++;
  EXPECT_EQ(perm.end(), iter);
}

TEST_F(ListIteratorTest, test_iterator_temp_list_single) {
  TempList temp;
  temp.changes = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  auto iter = temp.begin();
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);

  iter++;
  EXPECT_EQ(temp.end(), iter);
}

TEST_F(ListIteratorTest, test_iterator_temp_list_nested) {
  TempList bottom;
  bottom.changes = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  TempList change1;
  change1.right = &bottom;
  change1.changes = { Value((INT_T)4) };

  TempList change2;
  change2.right = &change1;
  change2.changes = { Value((INT_T)5) };

  // iterate staring at change2 (latest change)
  auto iter = change2.begin();
  EXPECT_EQ(5, (*iter).value.ival);

  iter++;
  EXPECT_EQ(4, (*iter).value.ival);

  iter++;
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);

  iter++;
  EXPECT_EQ(bottom.end(), iter);

  // iterate staring at change1
  iter = change1.begin();
  EXPECT_EQ(4, (*iter).value.ival);

  iter++;
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);
}

TEST_F(ListIteratorTest, test_iterator_temp_list_with_perm_list) {
  PermList bottom;
  bottom.values = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  TempList change1;
  change1.right = &bottom;
  change1.changes = { Value((INT_T)4) };

  TempList change2;
  change2.right = &change1;
  change2.changes = { Value((INT_T)5) };

  // iterate staring at change2 (latest change)
  auto iter = change2.begin();
  EXPECT_EQ(5, (*iter).value.ival);

  iter++;
  EXPECT_EQ(4, (*iter).value.ival);

  iter++;
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);

  iter++;
  EXPECT_EQ(bottom.end(), iter);

  // iterate staring at change1
  iter = change1.begin();
  EXPECT_EQ(4, (*iter).value.ival);

  iter++;
  EXPECT_EQ(1, (*iter).value.ival);

  iter++;
  EXPECT_EQ(2, (*iter).value.ival);

  iter++;
  EXPECT_EQ(3, (*iter).value.ival);
}

TEST_F(ListIteratorTest, test_iterator_skip_with_perm_list) {
  PermList bottom;
  bottom.values = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  TempList change1;
  change1.right = &bottom;
  change1.skip = 1;

  std::cout <<"temlist "<<&change1 << " permlist "<<&bottom << std::endl;
  auto iter = change1.begin();
  EXPECT_EQ(2, (*iter).value.ival);

  iter.next();
  EXPECT_EQ(3, (*iter).value.ival);

  iter.next();
  EXPECT_TRUE(iter == change1.end());

  TempList change2;
  change2.right = &change1;
  change2.skip = 1;

  iter = change2.begin();
  EXPECT_EQ(3, (*iter).value.ival);

  iter.next();
  EXPECT_TRUE(iter == change2.end());

  TempList change3;
  change3.right = &change2;
  change3.skip = 1;

  iter = change3.begin();
  EXPECT_TRUE(iter == change3.end());

}


TEST_F(ListIteratorTest, test_iterator_eq_begin_end) {
  PermList list;
  list.values = { Value((INT_T)1), Value((INT_T)2), Value((INT_T)3) };

  EXPECT_TRUE(list.begin() != list.end());
}

class ListTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }
};

TEST_F(ListTest, test_list_eq_temp_list_empty_bottom) {
  TempList temp;
  temp.changes = { };

  TempList change1;
  change1.right = &temp;
  change1.changes = { Value((INT_T)1) };

  PermList perm;
  perm.values = { Value((INT_T)1) };

  EXPECT_TRUE(change1 == perm);
  EXPECT_TRUE(perm == perm);
  EXPECT_TRUE(change1 == change1);
  EXPECT_FALSE(change1 != perm);
  EXPECT_FALSE(temp == perm);
}
