#include "libparse/ast.h"

bool IntValue::equals(Value *other) {
  IntValue *other_cast = dynamic_cast<IntValue*>(other);
  if (other_cast == nullptr) {
    return false;
  } else {
    return value_ == other_cast->value_;
  }
}

AstNode::AstNode(NodeType t) {
    this->type = t;
   // DEBUG(this->to_str());
}

std::string AstNode::to_str() {
    return std::string("AStNode: ")+ node_type_names[type];
}


void AstNode::visit(AstVisitor &v) {
    v.visit_node(this);
}

bool AstNode::equals(AstNode *other) {
  return type == other->type;
}

AstListNode::AstListNode(NodeType type) : AstNode(type) {}

void AstListNode::add(AstNode* n) {
    this->nodes.push_back(n);
}

void AstListNode::visit(AstVisitor &v) {
    v.visit_node(this);
    for(AstNode *n : nodes) {
      n->visit(v);
    }
}

bool AstListNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  AstListNode *other_cast = static_cast<AstListNode*>(other);

  auto ast1_iter = nodes.begin();
  auto ast2_iter = other_cast->nodes.begin();
  
  while (ast1_iter < nodes.end() && ast2_iter < other_cast->nodes.end()) {
    if (!((*ast1_iter)->equals(*ast2_iter))) {
      return false;
    }
    ast1_iter += 1;
    ast2_iter += 1;
  }

  if (ast1_iter == nodes.cend() && ast2_iter == other_cast->nodes.cend()) {
    return true;
  } else {
    return false;
  }
}

AtomNode::AtomNode(Value *val) : AstNode(NodeType::ATOM) {
  val_ = val;
}

bool AtomNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  AtomNode *other_cast = static_cast<AtomNode*>(other);
  return val_->equals(other_cast->val_);
}

Expression::Expression(Expression *left, AtomNode *right) : AstNode(NodeType::EXPRESSION) {
  left_ = left;
  right_ = right;
}

void Expression::visit(AstVisitor &v) {
    v.visit_node(this);
    if (left_ != nullptr) {
      left_->visit(v);
    }
    right_->visit(v);
}

bool Expression::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  Expression *other_cast = static_cast<Expression*>(other);
  if (left_ == nullptr) {
    return other_cast->left_ == nullptr && right_->equals(other_cast->right_);
  } else {
    return left_->equals(other_cast->left_) && right_->equals(other_cast->right_);
  }
}


UpdateNode::UpdateNode(Expression *expr) : AstNode(NodeType::UPDATE) {
  expr_ = expr;
}

void UpdateNode::visit(AstVisitor &v) {
    v.visit_node(this);
    expr_->visit(v);
}

bool UpdateNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  UpdateNode *other_cast = static_cast<UpdateNode*>(other);
  return expr_->equals(other_cast->expr_);
}

UnaryNode::UnaryNode(NodeType type, AstNode *child) : AstNode(type) {
  child_ = child;
}

void UnaryNode::visit(AstVisitor &v) {
    v.visit_node(this);
    child_->visit(v);
}

bool UnaryNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  UnaryNode *other_cast = static_cast<UnaryNode*>(other);
  return child_->equals(other_cast->child_);
}
