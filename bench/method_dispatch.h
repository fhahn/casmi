class AstNode {
    private:
        int line_number;
        int column;
        NodeType type;

    public:
        AstNode(NodeType t);
        virtual ~AstNode();
        virtual bool execute_virtual();
};

class AstListNode: public AstNode {
    private:
        std::vector<AstNode*> nodes;

    public:
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
  private:
    INT_T val_;

  public:
    IntAtom(INT_T val);
    ~IntAtom();
};


class Expression : public AstNode {
  private:
    Expression *left_;
    AtomNode *right_;

  public:
    Expression(Expression *left, AtomNode *right);
    virtual ~Expression();
};

class UpdateNode: public AstNode {
  private:
    Expression *expr_;

  public:
    UpdateNode(Expression *expr);
    virtual ~UpdateNode();
};
