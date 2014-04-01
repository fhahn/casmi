#ifndef CASMI_LIBPARSE_VISITOR_H
#define CASMI_LIBPARSE_VISITOR_H

#include "libutil/exceptions.h"
#include "libsyntax/ast.h"

class AstNode;

template<class T, class V> class AstWalker {
  public:
    T& visitor;


    AstWalker(T& v) : visitor(v) {}

    void walk_specification(AstNode *spec) {
      if (spec->node_type_ == NodeType::BODY_ELEMENTS) {
        visitor.visit_specification(spec);
        walk_body_elements(reinterpret_cast<AstListNode*>(spec));
      }
    }

    void walk_body_elements(AstListNode *body_elements) {
      for (auto e : body_elements->nodes) {
        switch(e->node_type_) {
          case NodeType::PROVIDER: {break;} // TODO implement
          case NodeType::OPTION: {break;} // TODO implement
          case NodeType::ENUM: {break;} // TODO implement
          case NodeType::FUNCTION: {break;} // TODO implement
          case NodeType::DERIVED: {break;} // TODO implement
          case NodeType::RULE: {
            walk_rule(reinterpret_cast<RuleNode*>(e));
            break; 
          } 
          case NodeType::INIT: {break;} // TODO implement
          default: {
            throw RuntimeException(
              std::string("Invalid node type: ")+
              type_to_str(e->node_type_)+
              std::string(" at ")+
              e->location_str());
          }
        }
      }
      visitor.visit_body_elements(body_elements);
    }

    void walk_rule(RuleNode *rule) {
      visitor.visit_rule(rule);
      walk_statement(rule->child_);
    }

    void walk_statement(AstNode *stmt) {
      switch(stmt->node_type_) {
        case NodeType::PARBLOCK: {
          walk_parblock(reinterpret_cast<UnaryNode*>(stmt));
          break;
        }
        case NodeType::UPDATE: {
          walk_update(reinterpret_cast<UpdateNode*>(stmt));
          break;
        }
        default: {
            throw RuntimeException(
              std::string("Invalid node type: ")+
              type_to_str(stmt->node_type_)+
              std::string(" at ")+
              stmt->location_str());
        }
      }
    }

    void walk_parblock(UnaryNode *parblock) {
      visitor.visit_parblock(parblock);
      walk_statements(reinterpret_cast<AstListNode*>(parblock->child_));
    }

    void walk_statements(AstListNode *stmts) {
      visitor.visit_statements(stmts);
      for (auto stmt: stmts->nodes) {
        walk_statement(stmt);
      }
    }

    void walk_update(UpdateNode *update) {
      V v = walk_expression(update->expr_);
      visitor.visit_update(update, v);
    }

    V walk_expression(Expression *expr) {
      if (expr->left_ != nullptr && expr->right_ != nullptr) {
        V v1 = walk_expression(expr->left_);
        V v2 = walk_atom(expr->right_);
        return visitor.visit_expression(expr, v1, v2);
      }

      if (expr->left_ != nullptr) {
        V v = walk_expression(expr->left_);

        return visitor.visit_expression_single(expr, v);
      }

      if (expr->right_ != nullptr) {
        V v = walk_atom(expr->right_);

        return visitor.visit_expression_single(expr, v);
      }

      throw RuntimeException("Invalid expression structure");
    }

    V walk_atom(AtomNode *atom) {
      switch(atom->node_type_) {
        case NodeType::INT_ATOM: {
          return visitor.visit_int_atom(reinterpret_cast<IntAtom*>(atom));
          break;
        }
        default: {
          throw RuntimeException("Invalid atom type");
        }
      }
    }


};

#endif
