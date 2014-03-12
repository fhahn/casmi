// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <string>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/parser.tab.h"

extern casmi_driver *global_driver;

class ParserTest: public ::testing::Test {
  protected:
    virtual void SetUp() { global_driver = &driver_; }

    casmi_string_driver driver_;
};

TEST_F(ParserTest, parse_simple) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "    y := 5 - 10\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  AstListNode *stmts = new AstListNode(NodeType::STATEMENTS);
  stmts->add(new UpdateNode(
        new Expression(
            new Expression(nullptr, create_atom(1)),
            create_atom(2))
  ));
  stmts->add(new UpdateNode(
        new Expression(
            new Expression(nullptr, create_atom(5)),
            create_atom(10))
  ));


  AstListNode *ast = new AstListNode(NodeType::BODY_ELEMENTS);
  ast->add(new AstNode(NodeType::INIT));
  ast->add(new UnaryNode(NodeType::RULE, new UnaryNode(NodeType::PARBLOCK, stmts)));
  EXPECT_EQ(true, root->equals(ast));

  delete ast;
  delete root;
}

TEST_F(ParserTest, parse_error) {
  std::string test = "init\n";
  EXPECT_EQ(nullptr, driver_.parse(test));
}

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
