#include <iostream>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/lexer_helpers.h"

#include "libparse/parser.tab.h"


class LexerHelpersTest: public ::testing::Test {
  protected:
    virtual void SetUp() { }

    casmi_driver driver;
    yy::location loc;
};

// tests convert_to_long with a valid string, base 10
TEST_F(LexerHelpersTest, convert_to_long_10_ok) {

  EXPECT_EQ(200, convert_to_long("200", 10, driver, loc));
}

// tests convert_to_long with a valid string, but the number is too big
// for long, base 10
TEST_F(LexerHelpersTest, convert_to_long_out_of_range) {
  ASSERT_THROW(
    convert_to_long("99999999999999999999999999999999", 10, driver, loc),
    std::out_of_range
  );
}

// tests convert_to_long with a string containing invalid characters (X), base 10
TEST_F(LexerHelpersTest, convert_to_long_invalid_string) {
  ASSERT_THROW(
    convert_to_long("10X0", 10, driver, loc),
    std::out_of_range
  );
}

// tests convert_to_long with a valid string, base 16
TEST_F(LexerHelpersTest, convert_to_long_16_ok) {
  EXPECT_EQ(512, convert_to_long("200", 16, driver, loc));
  EXPECT_EQ(26, convert_to_long("1A", 16, driver, loc));
  EXPECT_EQ(26, convert_to_long("1a", 16, driver, loc));
}

// tests convert_to_long with a valid string, but the number is too big
// for long, base 10
TEST_F(LexerHelpersTest, convert_to_long_16_out_of_range) {
  ASSERT_THROW(
    convert_to_long("99999999999999999999999999999999", 10, driver, loc),
    std::out_of_range
  );
}

// tests convert_to_long with a string containing invalid characters (X), base 10
TEST_F(LexerHelpersTest, convert_to_long_16_invalid_string) {
  ASSERT_THROW(
    convert_to_long("10X0", 10, driver, loc),
    std::out_of_range
  );
}
