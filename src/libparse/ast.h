#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"
#include "libparse/visitor.h"

class Value {
  public:
    virtual bool equals(Value *other) = 0;
};

class IntValue: public Value {
  private:
    int value_;
  public:
    IntValue(int v) { this->value_ = v; }
    virtual bool equals(Value *other);
};


// TODO enum class, but how to print?
enum NodeType { ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION, EXPRESSION, UPDATE, STATEMENT, PARBLOCK, STATEMENTS};

static const char* node_type_names[] = { "ATOM", "INIT", "BODY_ELEMENTS", "PROVIDER",  "OPTION", "ENUM", "FUNCTION", "DERIVED", "RULE", "SPECIFICATION", "EXPRESSION", "UPDATE", "STATEMENT", "PARBLOCK", "STATEMENTS"};

class AstVisitor;

class AstNode {
    private:
        int line_number;
        int column;
        NodeType type;

    public:
        AstNode(NodeType t);
        virtual ~AstNode();
        virtual std::string to_str();
        virtual void visit(AstVisitor &v);
        virtual bool equals(AstNode *other);
};

class AstListNode: public AstNode {
    private:
        std::vector<AstNode*> nodes;

    public:
        AstListNode(NodeType type);
        virtual ~AstListNode();
        void add(AstNode* n);
        virtual void visit(AstVisitor &v);
        virtual bool equals(AstNode *other);
};

class AtomNode: public AstNode {
  private:
    Value *val_;

  public:
    AtomNode(Value *val);
    virtual ~AtomNode();
    virtual bool equals(AstNode *other);
};

class Expression : public AstNode {
  private:
    Expression *left_;
    AtomNode *right_;

  public:
    Expression(Expression *left, AtomNode *right);
    virtual ~Expression();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
};

class UnaryNode: public AstNode {
  private:
    AstNode *child_;

  public:
    UnaryNode(NodeType type, AstNode *child);
    virtual ~UnaryNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
};

class UpdateNode: public AstNode {
  private:
    Expression *expr_;

  public:
    UpdateNode(Expression *expr);
    virtual ~UpdateNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
};

#endif
