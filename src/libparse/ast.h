#ifndef CASMI_AST_H
#define CASMI_AST_H

class AstNode {
    private:
        int line_number;
        int column;
};

class SpecificationNode : public AstNode {};

#endif
