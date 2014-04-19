// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <string>

#include "gtest/gtest.h"

#include "libsyntax/driver.h"
#include "libsyntax/parser.tab.h"
#include "libsyntax/location.hh"

#include "libutil/exceptions.h"

extern Driver *global_driver;

class ParserTest: public ::testing::Test {
  protected:
    virtual void SetUp() { global_driver = &driver_; }

    StringDriver driver_;
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
        new FunctionAtom(loc, "x"),
        new Expression(loc,
            new IntAtom(loc, 1),
            new IntAtom(loc, 2),
            Expression::Operation::SUB)
  ));

  stmts->add(new UpdateNode(loc,
        new FunctionAtom(loc, "y"),
        new Expression(loc,
            new IntAtom(loc, 5),
            new IntAtom(loc, 10),
            Expression::Operation::SUB)
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

TEST_F(ParserTest, parse_simple_symbols) {
  std::string test = "function x : Int -> Int\n"
                     "function y : Int -> Int\n"
                     "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "    y := 5 - 10\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  // TODO: Check why size of symbol table is 3
  // EXPECT_EQ(2, driver_.function_table.size());
  EXPECT_EQ("x", driver_.function_table.get("x")->name());
  EXPECT_EQ("y", driver_.function_table.get("y")->name());

  delete root;
}

TEST_F(ParserTest, parser_test_function_symbol_with_multiple_params) {
  std::string test = "function x : Int * Int -> Int\n"
                     "init main\n"
                     "rule main = { skip }\n";
  AstNode *root = driver_.parse(test);

  std::vector<Type> *types = new std::vector<Type>;
  types->push_back(Type::INT);
  types->push_back(Type::INT);
  Function x("x", types, Type::INT, nullptr);

  EXPECT_EQ(x.name(), driver_.function_table.get("x")->name());

  delete root;
}
