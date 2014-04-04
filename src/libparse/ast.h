#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"

#include "libparse/types.h"
#include "libparse/symbols.h"
#include "libparse/location.hh" // reuse bison's location class

// TODO enum class, but how to print?
enum NodeType { UNDEF_ATOM, INT_ATOM, DUMMY_ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION, EXPRESSION, UPDATE, STATEMENT, PARBLOCK, STATEMENTS, FUNCTION_ATOM};

static const char* node_type_names[] = { "UNDEF_ATOM", "INT_ATOM", "DUMMY_ATOM", "INIT", "BODY_ELEMENTS", "PROVIDER",  "OPTION", "ENUM", "FUNCTION", "DERIVED", "RULE", "SPECIFICATION", "EXPRESSION", "UPDATE", "STATEMENT", "PARBLOCK", "STATEMENTS"};
const std::string& type_to_str(NodeType t);

class casmi_driver;

class AstVisitor;
class SymbolUsage;

class AstNode {
    public:
        yy::location location;
        NodeType node_type_;

        Type type_;

        AstNode(NodeType node_type);
        AstNode(yy::location& loc, NodeType node_type);
        AstNode(yy::location& loc, NodeType node_type, Type type);
        virtual ~AstNode();
        virtual std::string to_str();
        virtual bool equals(AstNode *other);
        std::string location_str() const;
};

class AstListNode: public AstNode {
    public:
        std::vector<AstNode*> nodes;

        AstListNode(yy::location& loc, NodeType node_type);
        virtual ~AstListNode();
        void add(AstNode* n);
        virtual bool equals(AstNode *other);
};

class AtomNode: public AstNode {
  public:
    AtomNode() : AstNode(NodeType::DUMMY_ATOM) {}
    AtomNode(yy::location& loc, NodeType node_type, Type type) : AstNode(loc, node_type, type) {}
};

class IntAtom : public AtomNode {
  public:
    INT_T val_;

    IntAtom(yy::location& loc, INT_T val);
    virtual ~IntAtom();
    bool equals(AstNode *other);
};

class UndefAtom : public AtomNode {
  public:
    UndefAtom(yy::location& loc);
    bool equals(AstNode *other);
};



class FunctionAtom : public AtomNode {
  public:
    SymbolUsage *func_;

    FunctionAtom(yy::location& loc, SymbolUsage *val);
    virtual ~FunctionAtom();
    bool equals(AstNode *other);
};



class Expression : public AstNode {
  public:
    Expression *left_;
    AtomNode *right_;

    Expression(yy::location& loc, Expression *left, AtomNode *right);
    virtual ~Expression();
    virtual bool equals(AstNode *other);
};

class UnaryNode: public AstNode {
  public:
    AstNode *child_;

    UnaryNode(yy::location& loc, NodeType node_type, AstNode *child);
    virtual ~UnaryNode();
    virtual bool equals(AstNode *other);
};

class RuleNode: public UnaryNode {
  public:
    RuleNode(yy::location& loc, AstNode *child, const std::string &name);
    const std::string name;
};

class UpdateNode: public AstNode {
  public:
    SymbolUsage *sym_;
    Expression *expr_;

    UpdateNode(yy::location& loc, SymbolUsage *sym, Expression *expr);
    virtual ~UpdateNode();
    virtual bool equals(AstNode *other);
};


AtomNode* create_atom(yy::location& loc, INT_T val);
AtomNode* create_atom(yy::location& loc, SymbolUsage *val);
AtomNode* create_atom(yy::location& loc);
#endif
