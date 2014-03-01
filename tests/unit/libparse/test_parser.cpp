#include <string>
#include <stdexcept>

#include "gtest/gtest.h"

#include "libparse/driver.h"
#include "libparse/lexer_helpers.h"
#include "libparse/visitor.h"

#include "libparse/parser.tab.h"

extern casmi_driver *global_driver;

bool compare_ast(AstNode *ast1, AstNode *ast2) {
  SerializeVisitor v1, v2;
  ast1->visit(v1);
  ast2->visit(v2);

  auto ast1_iter = v1.items.begin();
  auto ast1_end = v1.items.end();

  auto ast2_iter = v2.items.begin();
  auto ast2_end = v2.items.end();
  
  while (ast1_iter < ast1_end && ast2_iter < ast2_end) {
    if (!((*ast1_iter)->equals(*ast2_iter))) {
      return false;
    }
    ast1_iter += 1;
    ast2_iter += 1;
  }

  if (ast1_iter == ast1_end && ast2_iter == ast2_end) {
    return true;
  } else {
    return false;
  }
}


TEST(CompareTest, test_one_node_equal) {
  AstNode ast1(NodeType::INIT);
  AstNode ast2(NodeType::INIT);

  EXPECT_EQ(true, compare_ast(&ast1, &ast2));
}

TEST(CompareTest, test_one_node_not_equal) {
  AstNode ast1(NodeType::ATOM);
  AstNode ast2(NodeType::INIT);

  EXPECT_EQ(false, compare_ast(&ast1, &ast2));
}

TEST(CompareTest, test_list_two_nodes) {
  AstListNode ast1(NodeType::BODY_ELEMENTS);
  AstNode ast1_1(NodeType::ATOM);
  ast1.add(&ast1_1);
  AstNode ast1_2(NodeType::INIT);
  ast1.add(&ast1_2);

  AstListNode ast2(NodeType::BODY_ELEMENTS);
  AstNode ast2_1(NodeType::ATOM);
  ast2.add(&ast2_1);
  AstNode ast2_2(NodeType::INIT);
  ast2.add(&ast2_2);

  EXPECT_EQ(true, ast1.equals(&ast2));
}

TEST(CompareTest, test_list_two_nodes_not_equals) {
  AstListNode ast1(NodeType::BODY_ELEMENTS);
  AstNode ast1_1(NodeType::ATOM);
  ast1.add(&ast1_1);
  AstNode ast1_2(NodeType::INIT);
  ast1.add(&ast1_2);

  AstListNode ast2(NodeType::BODY_ELEMENTS);
  AstNode ast2_1(NodeType::ATOM);
  ast2.add(&ast2_1);
  AstNode ast2_2(NodeType::RULE);
  ast2.add(&ast2_2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

TEST(CompareTest, test_list_length_unequal) {
  AstListNode ast1(NodeType::BODY_ELEMENTS);
  AstNode ast1_1(NodeType::ATOM);
  ast1.add(&ast1_1);

  AstListNode ast2(NodeType::BODY_ELEMENTS);
  AstNode ast2_1(NodeType::ATOM);
  ast2.add(&ast2_1);
  AstNode ast2_2(NodeType::RULE);
  ast2.add(&ast2_2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

TEST(CompareTest, test_expression) {
  IntValue v1_1(10);
  IntValue v1_2(50);
  AtomNode a1_1(&v1_1);
  AtomNode a1_2(&v1_2);
  Expression e1(nullptr, &a1_1);
  Expression ast1(&e1, &a1_2);

  IntValue v2_1(10);
  IntValue v2_2(50);
  AtomNode a2_1(&v2_1);
  AtomNode a2_2(&v2_2);
  Expression e2(nullptr, &a2_1);
  Expression ast2(&e2, &a2_2);

  EXPECT_EQ(true, ast1.equals(&ast2));
}

TEST(CompareTest, test_expression_not_equal) {
  IntValue v1_1(10);
  IntValue v1_2(10); // this value differs from ast2
  AtomNode a1_1(&v1_1);
  AtomNode a1_2(&v1_2);
  Expression e1(nullptr, &a1_1);
  Expression ast1(&e1, &a1_2);

  IntValue v2_1(10);
  IntValue v2_2(50);
  AtomNode a2_1(&v2_1);
  AtomNode a2_2(&v2_2);
  Expression e2(nullptr, &a2_1);
  Expression ast2(&e2, &a2_2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

TEST(CompareTest, test_expression_wrong_balance) {
  IntValue v1(10);
  AtomNode a1(&v1);
  Expression e1(nullptr, &a1);
  Expression ast1(&e1, &a1);

  IntValue v2(10);
  AtomNode a2(&v2);
  Expression e2_1(nullptr, &a2);
  Expression e2_2(&e2_1, &a2);
  Expression ast2(&e2_2, &a2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

TEST(CompareTest, test_update_node) {
  IntValue v1(10);
  AtomNode a1(&v1);
  Expression e1_1(nullptr, &a1);
  Expression e1_2(&e1_1, &a1);
  UpdateNode ast1(&e1_2);

  IntValue v2(10);
  AtomNode a2(&v2);
  Expression e2_1(nullptr, &a2);
  Expression e2_2(&e2_1, &a2);
  UpdateNode ast2(&e2_2);

  EXPECT_EQ(true, ast1.equals(&ast2));
}

TEST(CompareTest, test_update_node_not_equal) {
  IntValue v1(10);
  AtomNode a1(&v1);
  Expression e1_1(nullptr, &a1);
  UpdateNode ast1(&e1_1);

  IntValue v2(10);
  AtomNode a2(&v2);
  Expression e2_1(nullptr, &a2);
  Expression e2_2(&e2_1, &a2);
  UpdateNode ast2(&e2_2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

TEST(CompareTest, test_unary_node) {
  IntValue v1(10);
  AtomNode a1(&v1);
  Expression e1(nullptr, &a1);
  UnaryNode ast1(NodeType::ENUM, &e1);

  IntValue v2(10);
  AtomNode a2(&v2);
  Expression e2(nullptr, &a2);
  UnaryNode ast2(NodeType::ENUM, &e2);

  EXPECT_EQ(true, ast1.equals(&ast2));
}

TEST(CompareTest, test_unary_node_wrong_type) {
  IntValue v1(10);
  AtomNode a1(&v1);
  Expression e1(nullptr, &a1);
  UnaryNode ast1(NodeType::FUNCTION, &e1);

  IntValue v2(10);
  AtomNode a2(&v2);
  Expression e2(nullptr, &a2);
  UnaryNode ast2(NodeType::ENUM, &e2);

  EXPECT_EQ(false, ast1.equals(&ast2));
}

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
