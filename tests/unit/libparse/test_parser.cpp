#include <string>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/lexer_helpers.h"

#include "libparse/parser.tab.h"

extern casmi_driver *global_driver;

class ParserTest: public ::testing::Test {
  protected:
    virtual void SetUp() { global_driver = &driver_; }

    casmi_string_driver driver_;
};
// tests convert_to_long with a valid string, base 10
TEST_F(ParserTest, parse_simple) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "}\n";
  AstNode *root = driver_.parse(test);
  PrintVisitor v;
  root->visit(v);
 
}

TEST_F(ParserTest, parse_error) {
  std::string test = "init\n";
  EXPECT_EQ(nullptr, driver_.parse(test));
}
