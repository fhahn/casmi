#include "libparse/ast.h"


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


AstListNode::AstListNode() : AstNode(NodeType::BODY_ELEMENTS) {}

void AstListNode::add(AstNode* n) {
    this->nodes.push_back(n);
}

void AstListNode::visit(AstVisitor &v) {
    v.visit_node(this);
    for(AstNode *n : nodes) {
      n->visit(v);
    }
}

AtomNode::AtomNode(Value *val) : AstNode(NodeType::ATOM) {
  val_ = val;
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
    if (right_ != nullptr) {
      right_->visit(v);
    }
}

UpdateNode::UpdateNode(Expression *expr) : AstNode(NodeType::UPDATE) {
  expr_ = expr;
}

void UpdateNode::visit(AstVisitor &v) {
    v.visit_node(this);
    expr_->visit(v);
}

UnaryNode::UnaryNode(NodeType type, AstNode *child) : AstNode(type) {
  child_ = child;
}

void UnaryNode::visit(AstVisitor &v) {
    v.visit_node(this);
    child_->visit(v);
}
