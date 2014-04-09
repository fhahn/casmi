// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "gtest/gtest.h"

#include "libsyntax/driver.h"
#include "libsyntax/parser.tab.h"
#include "libsyntax/location.hh"


yy::location loc;

//
// Tests for equals() method of AST nodes
//
TEST(EqualsTest, test_list_two_nodes) {
  AstListNode *ast1 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(loc, NodeType::PROVIDER));
  ast1->add(new AstNode(loc, NodeType::INIT));

  AstListNode *ast2 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(loc, NodeType::PROVIDER));
  ast2->add(new AstNode(loc, NodeType::INIT));

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));
  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_list_two_nodes_not_equals) {
  AstListNode *ast1 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(loc, NodeType::PROVIDER));
  ast1->add(new AstNode(loc, NodeType::INIT));

  AstListNode *ast2 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(loc, NodeType::PROVIDER));
  ast2->add(new AstNode(loc, NodeType::RULE));

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_list_length_unequal) {
  AstListNode *ast1 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast1->add(new AstNode(loc, NodeType::PROVIDER));
  ast1->add(new AstNode(loc, NodeType::INIT));

  AstListNode *ast2 = new AstListNode(loc, NodeType::BODY_ELEMENTS);
  ast2->add(new AstNode(loc, NodeType::PROVIDER));
  ast1->add(new AstNode(loc, NodeType::INIT));
  ast1->add(new AstNode(loc, NodeType::INIT));

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_expression) {
  Expression *ast1 = new Expression(loc, 
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
      create_atom(loc, 50),
      Expression::Operation::ADD
  );

  Expression *ast2 = new Expression(loc, 
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
      create_atom(loc, 50),
      Expression::Operation::ADD
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}


TEST(EqualsTest, test_expression_not_equal) {
  Expression *ast1 = new Expression(loc, 
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
      create_atom(loc, 10), // this values differs from ast2
      Expression::Operation::ADD
  );

  Expression *ast2 = new Expression(loc, 
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
      create_atom(loc, 50),
      Expression::Operation::ADD
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_expression_wrong_balance) {
  Expression *ast1 = new Expression(loc, 
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
      create_atom(loc, 10),
      Expression::Operation::ADD
  );

  Expression *ast2 = new Expression(loc, 
      new Expression(loc, 
          new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
          create_atom(loc, 10),
          Expression::Operation::ADD
      ),
      create_atom(loc, 50),
      Expression::Operation::ADD
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_update_node) {
  UpdateNode *ast1 = new UpdateNode(loc, 
      new SymbolUsage(loc, "x"),
      new Expression(loc, 
          new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
          create_atom(loc, 50),
          Expression::Operation::SUB
        )
  );

  UpdateNode *ast2 = new UpdateNode(loc, 
      new SymbolUsage(loc, "x"),
      new Expression(loc, 
          new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
          create_atom(loc, 50),
          Expression::Operation::SUB
        )
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_update_node_not_equal) {
  UpdateNode *ast1 = new UpdateNode(loc, 
      new SymbolUsage(loc, "x"),
      new Expression(loc, 
          new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP),
          create_atom(loc, 50),
          Expression::Operation::SUB
        )
  );

  UpdateNode *ast2 = new UpdateNode(loc, 
      new SymbolUsage(loc, "x"),
      new Expression(loc, 
          nullptr,
          create_atom(loc, 50),
          Expression::Operation::SUB
      )
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_unary_node) {
  UnaryNode *ast1 = new UnaryNode(loc, NodeType::ENUM,
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP)
  );

  UnaryNode *ast2 = new UnaryNode(loc, NodeType::ENUM,
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP)
  );

  EXPECT_EQ(true, ast1->equals(ast2));
  EXPECT_EQ(true, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}

TEST(EqualsTest, test_unary_node_wrong_type) {
  UnaryNode *ast1 = new UnaryNode(loc, NodeType::ENUM,
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP)
  );

  UnaryNode *ast2 = new UnaryNode(loc, NodeType::FUNCTION,
      new Expression(loc, nullptr, create_atom(loc, 10), Expression::Operation::NOP)
  );

  EXPECT_EQ(false, ast1->equals(ast2));
  EXPECT_EQ(false, ast2->equals(ast1));

  delete ast1;
  delete ast2;
}
