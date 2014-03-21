class AstNode {
    public:
        int line_number;
        int column;
        NodeType type;

        AstNode(NodeType t);
        virtual ~AstNode();
        virtual bool execute_virtual();
};

class AstListNode: public AstNode {
    public:
        std::vector<AstNode*> nodes;

        AstListNode(NodeType type);
        virtual ~AstListNode();
        virtual bool execute_virtual();
};

class AtomNode: public AstNode {
  public:
    AtomNode() : AstNode(NodeType::DUMMY_ATOM) {}
    AtomNode(NodeType t) : AstNode(t) {}
};

class IntAtom : public AtomNode {
  public:
    INT_T val_;

    IntAtom(INT_T val);
    ~IntAtom();
};


class Expression : public AstNode {
  public:
    Expression *left_;
    AtomNode *right_;

    Expression(Expression *left, AtomNode *right);
    virtual ~Expression();
};

class UpdateNode: public AstNode {
  public:
    Expression *expr_;

    UpdateNode(Expression *expr);
    virtual ~UpdateNode();
};
