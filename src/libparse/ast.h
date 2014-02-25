#ifndef CASMI_AST_H
#define CASMI_AST_H

#include <string>
#include <vector>

#include "macros.h"
#include "libparse/visitor.h"

// TODO enum class, but how to print?
enum NodeType { ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION };

static const char* node_type_names[] = { "ATOM", "INIT", "BODY_ELEMENTS", "PROVIDER",  "OPTION", "ENUM", "FUNCTION", "DERIVED", "RULE", "SPECIFICATION"};

class AstVisitor;

class AstNode {
    private:
        int line_number;
        int column;
        NodeType type;

    public:
        AstNode(NodeType t);
        virtual std::string to_str();
        virtual void visit(AstVisitor &v);
};

class AstListNode: public AstNode {
    private:
        std::vector<AstNode*> nodes;

    public:
        AstListNode();
        void add(AstNode* n);
        virtual void visit(AstVisitor &v);
};

class Value {};


class IntValue: public Value {
    private:
        int value;
    public:
        IntValue(int v) { this->value = v; DEBUG(v); }
        IntValue() { this->value = 0; }
};

#endif
