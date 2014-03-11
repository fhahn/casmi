#include <cassert>

#include <iostream>

#include "libparse/visitor.h"

void PrintVisitor::visit_node(AstNode *n) {
    std::cout << n->to_str() << std::endl;
}

void LambdaVisitor::visit_node(AstNode *n) {
  assert(func_(n));
}
