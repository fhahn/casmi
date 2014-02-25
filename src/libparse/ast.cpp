#include "libparse/ast.h"


AstNode::AstNode(NodeType t) {
    this->type = t; DEBUG(this->to_str());
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
       v.visit_node(n);
    }
}
