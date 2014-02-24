#ifndef CASMI_AST_H
#define CASMI_AST_H

class AstNode {
    private:
        int line_number;
        int column;
};

class Value {};


class IntValue: public Value {
    private:
        int value;
    public:
        IntValue(int v) { this->value = v; }
        IntValue() { this->value = 0; }
};

#endif
