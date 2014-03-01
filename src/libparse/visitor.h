#ifndef CASMI_LIBPARSE_VISITOR_H
#define CASMI_LIBPARSE_VISITOR_H

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

class SerializeVisitor: public AstVisitor {
  public:
    std::vector<AstNode*> items;
    void visit_node(AstNode *node);
};

#endif
