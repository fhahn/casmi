// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <iostream>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libinterpreter/value.h"
#include "libinterpreter/symbolic.h"


using namespace symbolic;

class CheckConditionTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }
};

TEST_F(CheckConditionTest, check_eq_true) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)20);

  symbolic_condition p1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_eq_false) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)10);

  symbolic_condition p1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_eq_neq_false) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)20);

  symbolic_condition p1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_neq_true) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)20);

  symbolic_condition p1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition p2(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_neq_true_2) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)30);

  symbolic_condition k1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition k2(&sym, &b, ExpressionOperation::NEQ);
  symbolic_condition p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1, &k2}, &p));
}

TEST_F(CheckConditionTest, check_neq_not_found_1) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)30);

  symbolic_condition k1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::NOT_FOUND, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_neq_eq_true) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);
  Value b((INT_T)30);

  symbolic_condition k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_neq_eq_false) {
  Value sym(TypeType::SYMBOL, 1);
  Value a((INT_T)20);

  symbolic_condition k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition p(&sym, &a, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}
