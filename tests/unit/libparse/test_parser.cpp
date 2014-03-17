// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <string>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/parser.tab.h"
#include "libparse/location.hh"

extern casmi_driver *global_driver;

class ParserTest: public ::testing::Test {
  protected:
    virtual void SetUp() { global_driver = &driver_; }

    casmi_string_driver driver_;
    yy::location loc;
};

TEST_F(ParserTest, parse_simple) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "    y := 5 - 10\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  AstListNode *stmts = new AstListNode(loc, NodeType::STATEMENTS);

  stmts->add(new UpdateNode(loc,
        new SymbolUsage(loc, "x"),
        new Expression(loc,
            new Expression(loc, nullptr, create_atom(loc, 1)),
            create_atom(loc, 2))
  ));

  stmts->add(new UpdateNode(loc,
        new SymbolUsage(loc, "y"),
        new Expression(loc,
            new Expression(loc, nullptr, create_atom(loc, 5)),
            create_atom(loc, 10))
  ));

  AstListNode *ast = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast->add(new AstNode(loc, NodeType::INIT));
  ast->add(new UnaryNode(loc, NodeType::RULE, new UnaryNode(loc, NodeType::PARBLOCK, stmts)));
  EXPECT_EQ(true, root->equals(ast));

  delete ast;
  delete root;
}

TEST_F(ParserTest, parse_error) {
  std::string test = "init\n";
  EXPECT_EQ(nullptr, driver_.parse(test));
}

/*
TEST_F(ParserTest, parse_simple_types) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "    y := 5 - 10\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  LambdaVisitor v([&] (AstNode *n) {
      if (n->node_type_ == NodeType::EXPRESSION) {
        return n->type_ == Type::INT;
      } else {
        return true;
      }
  });

  root->visit(v);

  delete root;
}
*/

/*
TEST_F(ParserTest, parse_simple_symbols) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "    y := 5 - 10\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  EXPECT_EQ(2, driver_.current_symbol_table->size());
  EXPECT_EQ("x", driver_.current_symbol_table->get("x")->name());

  delete root;
}
*/

TEST_F(ParserTest, parser_test_function_symbol_with_multiple_params) {
  std::string test = "function x : Int * Int -> Int\n"
                     "init main\n"
                     "rule main = { skip }\n";
  AstNode *root = driver_.parse(test);

  std::vector<Type> *types = new std::vector<Type>;
  types->push_back(Type::INT);
  types->push_back(Type::INT);
  Symbol x("x", types, Type::INT);

  EXPECT_EQ(true, x.equals(driver_.current_symbol_table->get("x")));

  delete root;
}
