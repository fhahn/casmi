#include <iostream>

#include "libparse/visitor.h"

void PrintVisitor::visit_node(AstNode *n) {
    std::cout << n->to_str() << std::endl;
}

void SerializeVisitor::visit_node(AstNode *n) {
  items.push_back(n);
}
