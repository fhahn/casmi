// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <iostream>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libinterpreter/execution_context.h"


class ExecutionContextTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }
};

// tests convert_to_long with a valid string, base 10
TEST_F(ExecutionContextTest, test_argumentskey_with_unordered_map) {
  std::unordered_map<ArgumentsKey, int> map;

  uint64_t data1[3] = {1,2,3};
  map[{&data1[0], 3}] =  55;

  uint64_t val = map[{&data1[0], 3}];
  EXPECT_EQ(55, val);

  ArgumentsKey k = {&data1[0], 2};
  ASSERT_THROW(map.at(k), std::out_of_range);

  uint64_t data2[3] = {1,2,2};
  k = {&data2[0], 3};
  ASSERT_THROW(map.at(k), std::out_of_range);

  uint64_t data3[3] = {1,2,3};
  k = {&data2[0], 3};
  EXPECT_EQ(55, val);
}
