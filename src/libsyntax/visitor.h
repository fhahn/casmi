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
          case NodeType::FUNCTION: {
            FunctionDefNode *func = reinterpret_cast<FunctionDefNode*>(e);
            std::vector<std::pair<V, V>> initializer_results;
            if (func->sym->intitializers_) {
              for (std::pair<ExpressionBase*, ExpressionBase*> p : *func->sym->intitializers_) {
                V first;
                if (p.first) {
                  first = walk_expression_base(p.first);
                } else {
                  UndefAtom foo = {p.second->location};
                  first = walk_atom(&foo);
                }
                initializer_results.push_back(
                    std::pair<V, V>(first, walk_expression_base(p.second))
                );
              }
            }

            visitor.visit_function_def(func, initializer_results);
            break;
          }
          case NodeType::DERIVED: {break;} // TODO implement
          case NodeType::RULE: {
            walk_rule(reinterpret_cast<RuleNode*>(e));
            break; 
          } 
          case NodeType::INIT: {
            visitor.visit_init(e);
            break;
          }
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
        case NodeType::PARBLOCK:
          walk_parblock(reinterpret_cast<UnaryNode*>(stmt));
          break;
        case NodeType::UPDATE:
          walk_update(reinterpret_cast<UpdateNode*>(stmt));
          break;
        case NodeType::ASSERT: {
          UnaryNode *assert = reinterpret_cast<UnaryNode*>(stmt);
          V v = walk_expression_base(reinterpret_cast<ExpressionBase*>(assert->child_));
          visitor.visit_assert(assert, v);
          break;
        }
        case NodeType::SKIP: break; // skip does nothing
        case NodeType::IFTHENELSE: {
          walk_ifthenelse(reinterpret_cast<IfThenElseNode*>(stmt));
          break;
        }
        case NodeType::CALL: {
          walk_call(reinterpret_cast<CallNode*>(stmt));
          break;
        }
        case NodeType::PRINT: {
          walk_print(reinterpret_cast<PrintNode*>(stmt));
          break;
        }
        default:
            throw RuntimeException(
              std::string("Invalid node type: ")+
              type_to_str(stmt->node_type_)+
              std::string(" at ")+
              stmt->location_str());
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
      V func_t = walk_function_atom(update->func);
      V expr_t = walk_expression_base(update->expr_);
      visitor.visit_update(update, func_t, expr_t);
    }

    void walk_call(CallNode *call) {
      if (call->ruleref == nullptr) {
        visitor.visit_call_pre(call);
      } else {
        V v = walk_expression_base(call->ruleref);
        visitor.visit_call_pre(call, v);
      }

      if (call->rule != nullptr) {
        std::vector<V> argument_results;
        if (call->arguments != nullptr) {
          for (ExpressionBase *e: *call->arguments) {
            argument_results.push_back(walk_expression_base(e));
          }
        }
        visitor.visit_call(call, argument_results);

        walk_rule(call->rule);

        visitor.visit_call_post(call);
      } else {
        DEBUG("rule not set!");
      }
    }

    void walk_print(PrintNode *node) {
      std::vector<V> argument_results;
      for (ExpressionBase *e: node->atoms) {
          argument_results.push_back(walk_expression_base(e));
        }
      visitor.visit_print(node, argument_results);
    }


    V walk_expression_base(ExpressionBase *expr) {
      if (expr->node_type_ == NodeType::EXPRESSION) {
        Expression *e = reinterpret_cast<Expression*>(expr);
        V v1 = walk_expression_base(e->left_);
        V v2 = walk_expression_base(e->right_);
        return visitor.visit_expression(e, v1, v2);
      } else {
        return walk_atom(reinterpret_cast<AtomNode*>(expr));
      }

      throw RuntimeException("Invalid expression structure");
    }

    void walk_ifthenelse(IfThenElseNode *n) {
      V cond = walk_expression_base(n->condition_);
      visitor.visit_ifthenelse(n, cond);
      walk_statement(n->then_);
      if (n->else_) {
        walk_statement(n->else_);
      }
    }

    V walk_function_atom(FunctionAtom *func) {
      std::vector<V> expr_results;
      if (func->arguments) {
        for (ExpressionBase* e : *func->arguments) {
          expr_results.push_back(walk_expression_base(e));
        }
      }
      return visitor.visit_function_atom(func, expr_results);
    }

    V walk_atom(AtomNode *atom) {
      switch(atom->node_type_) {
        case NodeType::INT_ATOM: {
          return visitor.visit_int_atom(reinterpret_cast<IntAtom*>(atom));
        }
        case NodeType::FLOAT_ATOM: {
          return visitor.visit_float_atom(reinterpret_cast<FloatAtom*>(atom));
        }
        case NodeType::UNDEF_ATOM: { 
          return visitor.visit_undef_atom(reinterpret_cast<UndefAtom*>(atom));
        }
        case NodeType::FUNCTION_ATOM: { 
          return walk_function_atom(reinterpret_cast<FunctionAtom*>(atom));
        }
        case NodeType::SELF_ATOM: {
          return visitor.visit_self_atom(reinterpret_cast<SelfAtom*>(atom));
        }
        case NodeType::RULE_ATOM: {
          return visitor.visit_rule_atom(reinterpret_cast<RuleAtom*>(atom));
        }
        case NodeType::BOOLEAN_ATOM: {
          return visitor.visit_boolean_atom(reinterpret_cast<BooleanAtom*>(atom));
        }
        case NodeType::STRING_ATOM: {
          return visitor.visit_string_atom(reinterpret_cast<StringAtom*>(atom));
        }
        default: {
          throw RuntimeException("Invalid atom type:"+type_to_str(atom->node_type_));
        }
      }
    }
};


#endif
