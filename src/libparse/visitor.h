#ifndef CASMI_LIBPARSE_VISITOR_H
#define CASMI_LIBPARSE_VISITOR_H

#include <functional>

#include "libparse/ast.h"

class AstNode;

class AstVisitor {
    public:
        virtual void visit_node(AstNode *node) = 0;
};


class PrintVisitor : public AstVisitor {
    public:
        void visit_node(AstNode *node);
};

class LambdaVisitor: public AstVisitor {
  std::function<bool (AstNode *node)> func_;

  public:
    LambdaVisitor(std::function<bool (AstNode *node)> func) { func_ = func; }
    void visit_node(AstNode *node);
};

#endif
