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
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)20);

  symbolic_condition_t p1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_eq_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)10);

  symbolic_condition_t p1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_eq_neq_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)20);

  symbolic_condition_t p1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition_t p2(&sym, &b, ExpressionOperation::EQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_neq_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)20);

  symbolic_condition_t p1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition_t p2(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&p1}, &p2));
}

TEST_F(CheckConditionTest, check_neq_true_2) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition_t k2(&sym, &b, ExpressionOperation::NEQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1, &k2}, &p));
}

TEST_F(CheckConditionTest, check_neq_not_found_1) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::NEQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::NOT_FOUND, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_neq_eq_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_neq_eq_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &a, ExpressionOperation::NEQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_eq_lessereq_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)20);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_eq_lessereq_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_lessereq_lessereq_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)40);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::LESSEREQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_lessereq_lessereq_not_found) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)70);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::LESSEREQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::NOT_FOUND, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_greater_lessereq_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)40);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::GREATER);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_greater_lessereq_not_found) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)60);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::GREATER);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::LESSEREQ);

  EXPECT_EQ(check_status_t::NOT_FOUND, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_eq_greater_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)30);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::GREATER);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_eq_greater_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)60);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::EQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::GREATER);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_lessereq_greater_false) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)60);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::LESSEREQ);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::GREATER);

  EXPECT_EQ(check_status_t::FALSE, check_condition({&k1}, &p));
}

TEST_F(CheckConditionTest, check_greater_greater_true) {
  value_t sym(new symbol_t(1));
  value_t a((INT_T)50);
  value_t b((INT_T)60);

  symbolic_condition_t k1(&sym, &a, ExpressionOperation::GREATER);
  symbolic_condition_t p(&sym, &b, ExpressionOperation::GREATER);

  EXPECT_EQ(check_status_t::TRUE, check_condition({&k1}, &p));
}
