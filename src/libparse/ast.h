#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"

#include "libparse/types.h"
#include "libparse/symbols.h"
#include "libparse/location.hh" // reuse bison's location class

// TODO enum class, but how to print?
enum NodeType { INT_ATOM, DUMMY_ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION, EXPRESSION, UPDATE, STATEMENT, PARBLOCK, STATEMENTS};

static const char* node_type_names[] = { "INT_ATOM", "DUMMY_ATOM", "INIT", "BODY_ELEMENTS", "PROVIDER",  "OPTION", "ENUM", "FUNCTION", "DERIVED", "RULE", "SPECIFICATION", "EXPRESSION", "UPDATE", "STATEMENT", "PARBLOCK", "STATEMENTS"};

class casmi_driver;

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
        virtual Type propagate_types(Type top, casmi_driver &driver);
};

class AstListNode: public AstNode {
    public:
        std::vector<AstNode*> nodes;

        AstListNode(NodeType node_type);
        virtual ~AstListNode();
        void add(AstNode* n);
        virtual void visit(AstVisitor &v);
        virtual bool equals(AstNode *other);
        virtual Type propagate_types(Type top, casmi_driver &driver);
};

class AtomNode: public AstNode {
  public:
    AtomNode() : AstNode(NodeType::DUMMY_ATOM) {}
    AtomNode(NodeType node_type, Type type) : AstNode(node_type, type) {}
};

class IntAtom : public AtomNode {
  public:
    INT_T val_;

    IntAtom(INT_T val);
    virtual ~IntAtom();
    bool equals(AstNode *other);
    virtual Type propagate_types(Type top, casmi_driver &driver);
};


class Expression : public AstNode {
  public:
    Expression *left_;
    AtomNode *right_;

    Expression(Expression *left, AtomNode *right);
    virtual ~Expression();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
    virtual Type propagate_types(Type top, casmi_driver &driver);
};

class UnaryNode: public AstNode {
  public:
    AstNode *child_;

    UnaryNode(NodeType node_type, AstNode *child);
    virtual ~UnaryNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
    virtual Type propagate_types(Type top, casmi_driver &driver);
};

class UpdateNode: public AstNode {
  public:
    SymbolUsage *sym_;
    Expression *expr_;

    UpdateNode(SymbolUsage *sym, Expression *expr);
    virtual ~UpdateNode();
    virtual void visit(AstVisitor &v);
    virtual bool equals(AstNode *other);
    virtual Type propagate_types(Type top, casmi_driver &driver);
};


AtomNode* create_atom(INT_T val);
#endif
