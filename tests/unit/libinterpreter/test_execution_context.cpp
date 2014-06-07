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

TEST_F(ExecutionContextTest, test_argumentskey_with_unordered_map) {

  Type t1(TypeType::INT);
  std::vector<Type*> types = {&t1, &t1, &t1};

  std::unordered_map<ArgumentsKey, int> map(0, {types}, {types});

  uint64_t data1[3] = {1,2,3};
  map[ArgumentsKey(&data1[0], 3, false, 0)] =  55;

  uint64_t val = map[ArgumentsKey(&data1[0], 3, false, 0)];
  EXPECT_EQ(55, val);

  uint64_t data2[3] = {1,2,2};
  ASSERT_THROW(map.at(ArgumentsKey(&data2[0], 3, false, 0)), std::out_of_range);

  uint64_t data3[3] = {1,2,3};
  EXPECT_EQ(55, map.at(ArgumentsKey(&data3[0], 3, false, 0)));
}
