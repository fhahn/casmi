// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <string>
#include <stdexcept>
#include <memory>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/lexer_helpers.h"
#include "libparse/visitor.h"

#include "libparse/parser.tab.h"

extern casmi_driver *global_driver;

TEST(CompareTest, test_list_two_nodes) {
  AstListNode *ast1 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(NodeType::PROVIDER));
  ast1->add(new AstNode(NodeType::INIT));

  AstListNode *ast2 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(NodeType::PROVIDER));
  ast2->add(new AstNode(NodeType::INIT));

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));
  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_list_two_nodes_not_equals) {
  AstListNode *ast1 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(NodeType::PROVIDER));
  ast1->add(new AstNode(NodeType::INIT));

  AstListNode *ast2 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(NodeType::PROVIDER));
  ast2->add(new AstNode(NodeType::RULE));

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_list_length_unequal) {
  AstListNode *ast1 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(NodeType::PROVIDER));
  ast1->add(new AstNode(NodeType::INIT));

  AstListNode *ast2 = new AstListNode(NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(NodeType::PROVIDER));
  ast1->add(new AstNode(NodeType::INIT));
  ast1->add(new AstNode(NodeType::INIT));

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_expression) {
  Expression *ast1 = new Expression(
      new Expression(nullptr, create_atom(10)),
      create_atom(50)
  );

  Expression *ast2 = new Expression(
      new Expression(nullptr, create_atom(10)),
      create_atom(50)
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}


TEST(CompareTest, test_expression_not_equal) {
  Expression *ast1 = new Expression(
      new Expression(nullptr, create_atom(10)),
      create_atom(10) // this values differs from ast2
  );

  Expression *ast2 = new Expression(
      new Expression(nullptr, create_atom(10)),
      create_atom(50)
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_expression_wrong_balance) {
  Expression *ast1 = new Expression(
      new Expression(nullptr, create_atom(10)),
      create_atom(10) // this values differs from ast2
  );

  Expression *ast2 = new Expression(
      new Expression(
          new Expression(nullptr, create_atom(10)),
          create_atom(10)
      ),
      create_atom(50)
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_update_node) {
  UpdateNode *ast1 = new UpdateNode(
      new Expression(
          new Expression(nullptr, create_atom(10)),
          create_atom(50)
        )
  );

  UpdateNode *ast2 = new UpdateNode(
      new Expression(
          new Expression(nullptr, create_atom(10)),
          create_atom(50)
        )
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_update_node_not_equal) {
  UpdateNode *ast1 = new UpdateNode(
      new Expression(
          new Expression(nullptr, create_atom(10)),
          create_atom(50)
        )
  );

  UpdateNode *ast2 = new UpdateNode(
      new Expression(
          nullptr,
          create_atom(50)
      )
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_unary_node) {
  UnaryNode *ast1 = new UnaryNode(NodeType::ENUM,
      new Expression(nullptr, create_atom(10))
  );

  UnaryNode *ast2 = new UnaryNode(NodeType::ENUM,
      new Expression(nullptr, create_atom(10))
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(CompareTest, test_unary_node_wrong_type) {
  UnaryNode *ast1 = new UnaryNode(NodeType::ENUM,
      new Expression(nullptr, create_atom(10))
  );

  UnaryNode *ast2 = new UnaryNode(NodeType::FUNCTION,
      new Expression(nullptr, create_atom(10))
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

class ParserTest: public ::testing::Test {
  protected:
    virtual void SetUp() { global_driver = &driver_; }

    casmi_string_driver driver_;
};

TEST_F(ParserTest, parse_simple) {
  std::string test = "init main\n"
                     "rule main = {\n"
                     "    x := 1 - 2\n"
                     "}\n";
  AstNode *root = driver_.parse(test);

  AstListNode *stmts = new AstListNode(NodeType::STATEMENTS);
  stmts->add(new UpdateNode(
        new Expression(
            new Expression(nullptr, create_atom(1)),
            create_atom(2))
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
