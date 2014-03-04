#include <hayai.hpp>

#include <cassert>
#include <vector>

#include "libparse/types.h"

enum class NodeType { INT_ATOM, DUMMY_ATOM, INIT, BODY_ELEMENTS, PROVIDER, OPTION, ENUM, FUNCTION, DERIVED, RULE, SPECIFICATION, EXPRESSION, UPDATE, STATEMENT, PARBLOCK, STATEMENTS};

class AstNode {
    public:
        NodeType type;
        AstNode(NodeType t) : type(t) {}
        virtual ~AstNode(){}
        virtual INT_T execute_virtual(std::vector<INT_T> &context) { return 0; }
};

class AtomNode: public AstNode {
  public:
    AtomNode() : AstNode(NodeType::DUMMY_ATOM) {}
    AtomNode(NodeType t) : AstNode(t) {}
};

class IntAtom : public AtomNode {
  public:
    INT_T val_;

    IntAtom(INT_T val) { val_ = val; }
    virtual ~IntAtom() {}

    INT_T value() { return val_; }
};


class Expression : public AstNode {
  public:
    Expression *left_;
    AtomNode *right_;

    Expression(Expression *left, AtomNode *right) : AstNode(NodeType::EXPRESSION) {
      left_ = left;
      right_ = right;
    }

    virtual ~Expression() { delete left_; delete right_; }

    virtual INT_T execute_virtual(std::vector<INT_T> &context) {
      INT_T tmp = 0;
      if (left_ != nullptr) {
        tmp = left_->execute_virtual(context);
      }

      IntAtom *ia = reinterpret_cast<IntAtom*>(right_);
      return tmp + ia->value();
    }

    inline INT_T execute_normal(std::vector<INT_T> &context) {
      INT_T tmp = 0;
      if (left_ != nullptr) {
        tmp = left_->execute_normal(context);
      }

      IntAtom *ia = reinterpret_cast<IntAtom*>(right_);
      return tmp + ia->value();
    }
};

class UpdateNode: public AstNode {
  public:
    Expression *expr_;
    size_t index_ = 0;

    UpdateNode(Expression *expr) : AstNode(NodeType::UPDATE) { expr_ = expr; }

    virtual ~UpdateNode() { delete expr_; }

    virtual INT_T execute_virtual(std::vector<INT_T> &context) {
      context[index_] = expr_->execute_virtual(context);
      return 0;
    }

    inline INT_T execute_normal(std::vector<INT_T> &context) {
      context[index_] = expr_->execute_normal(context);
      return 0;
    }
};


class AstListNode: public AstNode {
    public:
        std::vector<AstNode*> nodes;
        AstListNode(NodeType type) : AstNode(type) {}

        virtual ~AstListNode() {
          for (auto n : nodes) {
            delete n;
          }
          nodes.clear();
        }

        void add(AstNode* n) { nodes.push_back(n); }

        virtual INT_T execute_virtual(std::vector<INT_T> &context) {
          for (auto node: nodes) {
            node->execute_virtual(context);
          }
          return 0;
        }

        inline INT_T execute_normal(std::vector<INT_T> &context) {
          for (auto node: nodes) {
            switch (node->type) {
              case NodeType::UPDATE: {
                reinterpret_cast<UpdateNode*>(node)->execute_normal(context);
                break;
              }
              case NodeType::STATEMENT: {
                assert(false);
              }
            }
          }
          return 0;
        }
};

class AstFixture: public hayai::Fixture {
  public:
    virtual void SetUp() {
      ast = new AstListNode(NodeType::STATEMENTS);

      for(int i = 0; i < 10000; i++) {
        ast->add(new UpdateNode(new Expression(
            new Expression(new Expression(nullptr, new IntAtom(10)), new IntAtom(20)),
            new IntAtom(30)))
        );
      }

      context.push_back(0);
    }

    virtual void TearDown() {
      assert(context[0] == 60);
      delete ast;
    }

    AstListNode* ast;
    std::vector<INT_T> context;
};

BENCHMARK_F(AstFixture, VirtualFunctions, 10, 100) {
  ast->execute_virtual(context);
}

BENCHMARK_F(AstFixture, NormalFunctionsWithCast, 10, 100) {
  ast->execute_normal(context);
}


INT_T exec_expr(Expression *e) {
  INT_T tmp = 0;
  if (e->left_ != nullptr) {
    tmp = exec_expr(e->left_);
  }

  IntAtom *ia = reinterpret_cast<IntAtom*>(e->right_);
  return tmp + ia->value();
}

BENCHMARK_F(AstFixture, SwitchDispatch, 10, 100) {
  for(auto node : ast->nodes) {
    switch (node->type) {
      case NodeType::UPDATE: {
        UpdateNode *n = reinterpret_cast<UpdateNode*>(node);
        context[n->index_] = exec_expr(n->expr_);
        break;
      }
      case NodeType::STATEMENT: {
        assert(false);
      }
    }
  }

}
