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
  FLOAT_ATOM,
  SELF_ATOM,
  STRING_ATOM,
  RULE_ATOM,
  DUMMY_ATOM,
  BOOLEAN_ATOM,
  INIT,
  IFTHENELSE,
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
  STATEMENTS,
  SKIP,
  PARBLOCK,
  FUNCTION_ATOM,
  CALL,
  PRINT,
};

const std::string& type_to_str(NodeType t);

class AstVisitor;

// Forward declarations for libsyntax/symbols.h
class Function;
class Binding;

// Forward delclarations for this file
class Expression;
class ExpressionBase;

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


class UnaryNode: public AstNode {
  public:
    AstNode *child_;

    UnaryNode(yy::location& loc, NodeType node_type, AstNode *child);
    virtual ~UnaryNode();
    virtual bool equals(AstNode *other);
};


class RuleNode: public UnaryNode {
  public:
    const std::string name;
    const std::vector<Type> arguments;
    std::map<std::string, size_t> binding_offsets;

    RuleNode(yy::location& loc, AstNode *child, const std::string &name);
    RuleNode(yy::location& loc, AstNode *child, const std::string &name,
        const std::vector<Type>& args);
};


class FunctionDefNode: public AstNode {
  public:
    Function *sym;
    FunctionDefNode(yy::location& loc, Function *sym);
    ~FunctionDefNode();
};


class IfThenElseNode : public AstNode {
  public:
    ExpressionBase *condition_;
    AstNode *then_; // should always be a statement
    AstNode *else_; // should always be a statement

    IfThenElseNode(yy::location& loc, ExpressionBase *condition, AstNode *then,
                   AstNode *els);
};

class ExpressionBase : public AstNode {
  public:
    ExpressionBase(yy::location& loc, NodeType node_type, Type type) : AstNode(loc, node_type, type) {}

};

class AtomNode: public ExpressionBase {
  public:
    AtomNode(yy::location& loc, NodeType node_type, Type type) : ExpressionBase(loc, node_type, type) {}
};

class IntAtom : public AtomNode {
  public:
    INT_T val_;

    IntAtom(yy::location& loc, INT_T val);
    virtual ~IntAtom();
    bool equals(AstNode *other);
};

class FloatAtom : public AtomNode {
  public:
    FLOAT_T val_;

    FloatAtom(yy::location& loc, FLOAT_T val);
    virtual ~FloatAtom();
    bool equals(AstNode *other);
};


class UndefAtom : public AtomNode {
  public:
    UndefAtom(yy::location& loc);
    bool equals(AstNode *other);
};


class SelfAtom : public AtomNode {
  public:
    SelfAtom(yy::location& loc);
    bool equals(AstNode *other);
};


class BooleanAtom : public AtomNode {
  public:
    bool value;
    BooleanAtom(yy::location& loc, bool value);
    bool equals(AstNode *other);
};


class FunctionAtom : public AtomNode {
  public:
    Function *symbol;
    size_t binding_offset;
    const std::string name;
    std::vector<ExpressionBase*> *arguments;

    FunctionAtom(yy::location& loc, const std::string name);
    FunctionAtom(yy::location& loc, const std::string name,
                 std::vector<ExpressionBase*> *args);
 
    virtual ~FunctionAtom();
    bool equals(AstNode *other);
};


class RuleAtom : public AtomNode {
  public:
    RuleNode *rule;
    const std::string name;

    RuleAtom(yy::location& loc, const std::string&& name);
 
    virtual ~RuleAtom();
    bool equals(AstNode *other);
};


class StringAtom : public AtomNode {
  public:
    std::string string;

    StringAtom(yy::location& loc, std::string&& name);
 
    virtual ~StringAtom();
    bool equals(AstNode *other);
};


class Expression : public ExpressionBase {
  public:
    enum class Operation {
      ADD,
      SUB,
      MUL,
      DIV,
      MOD,
      RAT_DIV,
      EQ,
      NEQ,
      LESSER,
      GREATER,
      LESSEREQ,
      GREATEREQ,
      OR,
      XOR,
      AND,
      NOP
    };

    ExpressionBase *left_;
    ExpressionBase *right_;

    Operation op;

    Expression(yy::location& loc, ExpressionBase *left, ExpressionBase *right, Expression::Operation op);
    virtual ~Expression();
    virtual bool equals(AstNode *other);
};

std::string operator_to_str(const Expression::Operation op);

class UpdateNode: public AstNode {
  public:
    FunctionAtom *func;
    ExpressionBase *expr_;

    UpdateNode(yy::location& loc, FunctionAtom *func, ExpressionBase *expr);
    virtual ~UpdateNode();
    virtual bool equals(AstNode *other);
};

class CallNode: public AstNode {
  public:
    const std::string rule_name;
    RuleNode *rule;
    std::vector<ExpressionBase*> *arguments;
    ExpressionBase *ruleref;

    CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref);
    CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref,
             std::vector<ExpressionBase*> *args);
};

class PrintNode: public AstNode {
  public:
    std::vector<ExpressionBase*> atoms;

    PrintNode(yy::location& loc, const std::vector<ExpressionBase*> &atoms);
};
#endif
