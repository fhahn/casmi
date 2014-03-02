// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/parser.tab.h"

//
// Tests for equals() method of AST nodes
//
TEST(EqualsTest, test_list_two_nodes) {
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

TEST(EqualsTest, test_list_two_nodes_not_equals) {
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

TEST(EqualsTest, test_list_length_unequal) {
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

TEST(EqualsTest, test_expression) {
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


TEST(EqualsTest, test_expression_not_equal) {
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

TEST(EqualsTest, test_expression_wrong_balance) {
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

TEST(EqualsTest, test_update_node) {
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

TEST(EqualsTest, test_update_node_not_equal) {
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

TEST(EqualsTest, test_unary_node) {
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

TEST(EqualsTest, test_unary_node_wrong_type) {
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
