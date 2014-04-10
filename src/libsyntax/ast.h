#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"

#include "libsyntax/types.h"
#include "libsyntax/symbols.h"
#include "libsyntax/location.hh" // reuse bison's location class

enum class NodeType {
  ASSERT,
  UNDEF_ATOM,
  INT_ATOM,
  DUMMY_ATOM,
  INIT,
  BODY_ELEMENTS,
  PROVIDER,
  OPTION,
  ENUM,
  FUNCTION,
  DERIVED,
  RULE,
  SPECIFICATION,
  EXPRESSION,
  UPDATE,
  STATEMENT,
  PARBLOCK,
  STATEMENTS,
  FUNCTION_ATOM
};

const std::string& type_to_str(NodeType t);

class AstVisitor;
class Symbol;
class Expression;

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

class FunctionDefNode: public AstNode {
  public:
    Symbol *sym;
    FunctionDefNode(yy::location& loc, Symbol *sym);
    ~FunctionDefNode();
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
    Symbol *symbol;
    const std::string name;
    std::vector<Expression*> *arguments;

    FunctionAtom(yy::location& loc, const std::string name);
    FunctionAtom(yy::location& loc, const std::string name,
                 std::vector<Expression*> *args);
 
    virtual ~FunctionAtom();
    bool equals(AstNode *other);
};


class Expression : public AstNode {
  public:
    enum class Operation {
      ADD,
      SUB,
      NEQ,
      EQ,
      LESSER,
      GREATER,
      LESSEREQ,
      GREATEREQ,
      MUL,
      DIV,
      MOD,
      RAT_DIV,
      OR,
      XOR,
      AND,
      NOP
    };

    Expression *left_;
    AtomNode *right_;

    Operation op;

    Expression(yy::location& loc, Expression *left, AtomNode *right, Expression::Operation op);
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
    FunctionAtom *func;
    Expression *expr_;

    UpdateNode(yy::location& loc, FunctionAtom *func, Expression *expr);
    virtual ~UpdateNode();
    virtual bool equals(AstNode *other);
};

#endif
