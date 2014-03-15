#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"
#include "libparse/visitor.h"
#include "libparse/types.h"
#include "libparse/symbols.h"
#include "libparse/location.hh" // reuse bison's location class

// TODO enum class, but how to print?
enum NodeType { INT_ATOM, DUMMY_ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION, EXPRESSION, UPDATE, STATEMENT, PARBLOCK, STATEMENTS};

static const char* node_type_names[] = { "INT_ATOM", "DUMMY_ATOM", "INIT", "BODY_ELEMENTS", "PROVIDER",  "OPTION", "ENUM", "FUNCTION", "DERIVED", "RULE", "SPECIFICATION", "EXPRESSION", "UPDATE", "STATEMENT", "PARBLOCK", "STATEMENTS"};

class AstVisitor;
class SymbolUsage;

class AstNode {
    public:
        Type type_;
        NodeType node_type_;
        yy::location location;

        AstNode(NodeType node_type);
        AstNode(NodeType node_type, Type type);
        AstNode(NodeType node_type, yy::location& loc);
        virtual ~AstNode();
        virtual std::string to_str();
        virtual void visit(AstVisitor &v);
        virtual bool equals(AstNode *other);
};

class AstListNode: public AstNode {
    private:
        std::vector<AstNode*> nodes;

    public:
        AstListNode(NodeType node_type);
        virtual ~AstListNode();
        void add(AstNode* n);
        virtual void visit(AstVisitor &v);
        virtual bool equals(AstNode *other);
};

class AtomNode: public AstNode {
  public:
    AtomNode() : AstNode(NodeType::DUMMY_ATOM) {}
    AtomNode(NodeType node_type, Type type) : AstNode(node_type, type) {}
};

class IntAtom : public AtomNode {
  private:
    INT_T val_;

  public:
    IntAtom(INT_T val);
    ~IntAtom();
    bool equals(AstNode *other);
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
    UnaryNode(NodeType node_type, AstNode *child);
    virtual ~UnaryNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
};

class UpdateNode: public AstNode {
  private:
    SymbolUsage *sym_;
    Expression *expr_;

  public:
    UpdateNode(SymbolUsage *sym, Expression *expr);
    virtual ~UpdateNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
};


AtomNode* create_atom(INT_T val);
#endif
