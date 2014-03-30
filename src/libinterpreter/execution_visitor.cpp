#include "libinterpreter/execution_visitor.h"


ExecutionVisitor::ExecutionVisitor(AstNode *root, ExecutionContext& ctx) 
  : root_(root), context_(ctx) {}

void ExecutionVisitor::visit_update(UpdateNode *update, Value& val) {
  std::cout << "HALLO "<< val.value.ival <<"\n";
}


Value&& ExecutionVisitor::visit_expression(Expression *expr, Value &left_val, Value &right_val) {
  switch (left_val.type) {
    case Type::INT: {
      left_val.value.ival += right_val.value.ival;
      return std::move(left_val);
    }
    default: {
      throw std::string("KABOOM");
    }
  }
}

Value&& ExecutionVisitor::visit_expression_single(Expression *expr, Value &val) {
  return std::move(val);
}
