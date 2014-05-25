#ifndef CASMI_LIBPARSE_VISITOR_H
#define CASMI_LIBPARSE_VISITOR_H

#include "libutil/exceptions.h"
#include "libsyntax/ast.h"

class AstNode;

struct continuation_record_t {
  AstNode *caller;
  std::vector<AstNode*>::iterator next;
  std::vector<AstNode*>::iterator end;
};

template<class T, class V> class AstWalker {
  private:
      std::vector<AstNode*> tmp;

      void iterators_end() {
        current = tmp.end();
        current_end = tmp.end();
      }

  public:
    typedef std::vector<AstNode*>::iterator stmt_iter_type;
    T& visitor;

    std::vector<AstNode*>::iterator current;
    std::vector<AstNode*>::iterator current_end;


    AstWalker(T& v) : tmp(), visitor(v) {
      iterators_end();
    }

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
            walk_function_def(reinterpret_cast<FunctionDefNode*>(e));
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

    void walk_function_def(FunctionDefNode *def) {
      if (def->sym->type == Symbol::SymbolType::FUNCTION) {
        std::vector<std::pair<V, V>> initializer_results;
        if (def->sym->intitializers_) {
          for (std::pair<ExpressionBase*, ExpressionBase*> p : *def->sym->intitializers_) {
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

        visitor.visit_function_def(def, initializer_results);
      } else {
        visitor.visit_derived_def_pre(def);
        V v = walk_expression_base(def->sym->derived);
        visitor.visit_derived_def(def, v);
      }
    }

    void walk_rule(RuleNode *rule) {
      visitor.visit_rule(rule);
      walk_statement(rule->child_, true);
    }

    bool walk_statement(AstNode *stmt, bool single) {
      if (single) {
        iterators_end();
      }
      bool split = false;
      switch(stmt->node_type_) {
        case NodeType::SEQBLOCK: {
          visitor.continuations.push_back({stmt, ++stmt_iter_type(current), stmt_iter_type(current_end)});
          walk_seqblock(reinterpret_cast<UnaryNode*>(stmt));
          split = true;
          break;
        }
        case NodeType::PARBLOCK: {
          visitor.continuations.push_back({stmt, ++stmt_iter_type(current), stmt_iter_type(current_end)});
          walk_parblock(reinterpret_cast<UnaryNode*>(stmt));
          split = true;
          break;
        }
        case NodeType::UPDATE: {
          UpdateNode* up = reinterpret_cast<UpdateNode*>(stmt);
          DEBUG("WALK UPDATE");
          walk_update(up);
          break;
        }
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
          visitor.continuations.push_back({stmt, ++stmt_iter_type(current), stmt_iter_type(current_end)});
          walk_call(reinterpret_cast<CallNode*>(stmt));
          split = true;
          break;
        }
        case NodeType::PRINT: {
          walk_print(reinterpret_cast<PrintNode*>(stmt));
          break;
        }
        case NodeType::LET: {
          visitor.continuations.push_back({stmt, ++stmt_iter_type(current), stmt_iter_type(current_end)});
          walk_let(reinterpret_cast<LetNode*>(stmt));
          split = true;
          break;
        }
        case NodeType::POP: {
          walk_pop(reinterpret_cast<PopNode*>(stmt));
          break;
        }
        case NodeType::PUSH: {
          walk_push(reinterpret_cast<PushNode*>(stmt));
          break;
        }
        case NodeType::FORALL: {
          walk_forall(reinterpret_cast<ForallNode*>(stmt));
          split = true;
          break;
        }
        case NodeType::ITERATE: {
          walk_iterate(reinterpret_cast<UnaryNode*>(stmt));
          break;
        }
        case NodeType::CASE: {
          walk_case(reinterpret_cast<CaseNode*>(stmt));
          break;
        }
        case NodeType::UPDATE_DUMPS:
          walk_update_dumps(reinterpret_cast<UpdateNode*>(stmt));
          break;
        case NodeType::DIEDIE: {
            DiedieNode *node = reinterpret_cast<DiedieNode*>(stmt);
            if (node->msg) {
              visitor.visit_diedie(node, walk_expression_base(node->msg));
            } else {
              visitor.visit_diedie(node, V());
            }
          }
          break;
        default:
            throw RuntimeException(
              std::string("Invalid node type: ")+
              type_to_str(stmt->node_type_)+
              std::string(" at ")+
              stmt->location_str());
        }
      if (single) {
        walk_return();
      }
      return split;
    }

    void walk_return() {
      if (visitor.continuations.size() > 0) {
        continuation_record_t cont = visitor.continuations.back();
        visitor.continuations.pop_back();
        switch (cont.caller->node_type_) {
          case NodeType::CALL:
            visitor.visit_call_post(reinterpret_cast<CallNode*>(cont.caller));
            break;
          case NodeType::PARBLOCK:
            visitor.visit_parblock_post();
            break;
          case NodeType::SEQBLOCK:
            visitor.visit_seqblock_post();
            break;
          case NodeType::LET:
            visitor.visit_let_post(reinterpret_cast<LetNode*>(cont.caller));
            break;
          case NodeType::FORALL:
            visitor.visit_forall_post(reinterpret_cast<ForallNode*>(cont.caller));
            break;
        }
        if (cont.next < cont.end) {
          current = cont.next;
          current_end = cont.end;
          DEBUG("WALK CONTINUE ");
          walk_statements_continue();
        } else {
          DEBUG("WALK RETURN");
          walk_return();
        }
      }
    }

    void walk_seqblock(UnaryNode *parblock) {
      visitor.visit_seqblock(parblock);
      walk_statements(reinterpret_cast<AstListNode*>(parblock->child_));
    }

    void walk_parblock(UnaryNode *parblock) {
      visitor.visit_parblock(parblock);
      walk_statements(reinterpret_cast<AstListNode*>(parblock->child_));
    }

    void walk_statements(AstListNode *stmts) {
      visitor.visit_statements(stmts);
      current = stmts->nodes.begin();
      current_end = stmts->nodes.end();
      while (current < current_end) {
        if (walk_statement(*current, false)) {
          return;
        }
        current++;
      }
      walk_return();
    }

    void walk_statements_continue() {
      while (current < current_end) {
        if (walk_statement(*current, false)) {
          return;
        }
        current++;
      }
      walk_return();
    }


    void walk_update(UpdateNode *update) {
      DEBUG("WALK UPPPP");
      V expr_t = walk_expression_base(update->expr_);

      // we must walk the expression before walking update->func because it 
      // sets the list of arguments and we do not want the update->expr_ to
      // overwrite the value_list
      V func_t = walk_function_atom(update->func);
      visitor.visit_update(update, func_t, expr_t);
    }

    void walk_update_dumps(UpdateNode *update) {
      V expr_t = walk_expression_base(update->expr_);

      // we must walk the expression before walking update->func because it 
      // sets the list of arguments and we do not want the update->expr_ to
      // overwrite the value_list
      V func_t = walk_function_atom(update->func);
      visitor.visit_update_dumps(update, func_t, expr_t);
    }


    void walk_call(CallNode *call) {
      if (call->ruleref == nullptr) {
        visitor.visit_call_pre(call);
      } else {
        V v = walk_expression_base(call->ruleref);
        visitor.visit_call_pre(call, v);
      }

      // we must evaluate all arguments, to set correct offset for bindings
      std::vector<V> argument_results;
      if (call->arguments != nullptr) {
        for (ExpressionBase *e: *call->arguments) {
          argument_results.push_back(walk_expression_base(e));
        }
      }
      if (call->rule != nullptr) {
        visitor.visit_call(call, argument_results);
        DEBUG("\n CALL \n");
        walk_rule(call->rule);
      } else {
        DEBUG("rule not set!");
      }
    }

    void walk_print(PrintNode *node) {
      std::vector<V> argument_results;
      for (ExpressionBase *e: node->atoms) {
      DEBUG("VISIT_PRINT "<<e);
        argument_results.push_back(walk_expression_base(e));
      }
      DEBUG("GO VISIT");
      visitor.visit_print(node, argument_results);
    }

    void walk_let(LetNode *node) {
      V v = walk_expression_base(node->expr);
      visitor.visit_let(node, v);
      walk_statement(node->stmt, true);
    }

    void walk_pop(PopNode *node) {
      walk_function_atom(node->from);
      visitor.visit_pop(node);
    }

    void walk_push(PushNode *node) {
      V expr = walk_expression_base(node->expr);
      V atom = walk_function_atom(node->to);
      visitor.visit_push(node, expr, atom);
    }

    void walk_forall(ForallNode *node) {
      walk_expression_base(node->in_expr);
      visitor.visit_forall_pre(node);
      visitor.continuations.push_back({node, ++stmt_iter_type(current), stmt_iter_type(current_end)});
      walk_statement(node->statement, true);
    }

    void walk_iterate(UnaryNode* node) {
      visitor.visit_iterate(node);
      walk_statement(node->child_, false);
    }

    void walk_case(CaseNode *node) {
      std::vector<V> case_labels;
      for (auto& pair : node->case_list) {
        // pair.first == nullptr for default:
        if (pair.first) {
          case_labels.push_back(walk_atom(pair.first));
        }
        walk_statement(pair.second, false);
      }
      visitor.visit_case(node, walk_expression_base(node->expr), case_labels);
    }

    V walk_expression_base(ExpressionBase *expr) {
      if (expr->node_type_ == NodeType::EXPRESSION) {
        Expression *e = reinterpret_cast<Expression*>(expr);
        V v1 = walk_expression_base(e->left_);
        if (e->right_) {
          V v2 = walk_expression_base(e->right_);
          return visitor.visit_expression(e, v1, v2);
        } else {
          return visitor.visit_expression_single(e, v1);
        }
      } else {
        return walk_atom(reinterpret_cast<AtomNode*>(expr));
      }

      throw RuntimeException("Invalid expression structure");
    }

    void walk_ifthenelse(IfThenElseNode *n) {
      V cond = walk_expression_base(n->condition_);
      visitor.visit_ifthenelse(n, cond);
      walk_statement(n->then_, false);
      if (n->else_) {
        walk_statement(n->else_, false);
      }
    }

    V walk_function_atom(BaseFunctionAtom *func) {
      std::vector<V> expr_results;
      if (func->arguments) {
        for (ExpressionBase* e : *func->arguments) {
          expr_results.push_back(walk_expression_base(e));
        }
      }
      if (func->node_type_ == NodeType::BUILTIN_ATOM) {
         return visitor.visit_builtin_atom(reinterpret_cast<BuiltinAtom*>(func), expr_results);
      } else {
        FunctionAtom *func_a = reinterpret_cast<FunctionAtom*>(func);
        if (func_a->symbol_type == FunctionAtom::SymbolType::DERIVED) {
          visitor.visit_derived_function_atom_pre(func_a, expr_results);
          V expr = walk_expression_base(func_a->symbol->derived);
          return visitor.visit_derived_function_atom(func_a, expr);
        } else {
      DEBUG("WALK ATOM");
           return visitor.visit_function_atom(func_a, expr_results);
        }
      }
    }

    V walk_list_atom(ListAtom *atom) {
      std::vector<V> expr_results;
      if (atom->expr_list) {
        for (ExpressionBase* e : *atom->expr_list) {
          expr_results.push_back(walk_expression_base(e));
        }
      }
      return visitor.visit_list_atom(atom, expr_results);
    }

    V walk_atom(AtomNode *atom) {
      switch(atom->node_type_) {
        case NodeType::INT_ATOM: {
          return visitor.visit_int_atom(reinterpret_cast<IntAtom*>(atom));
        }
        case NodeType::FLOAT_ATOM: {
          return visitor.visit_float_atom(reinterpret_cast<FloatAtom*>(atom));
        }
        case NodeType::RATIONAL_ATOM: {
          return visitor.visit_rational_atom(reinterpret_cast<RationalAtom*>(atom));
        }
         case NodeType::UNDEF_ATOM: { 
          return visitor.visit_undef_atom(reinterpret_cast<UndefAtom*>(atom));
        }
        case NodeType::BUILTIN_ATOM:
        case NodeType::FUNCTION_ATOM: { 
          return walk_function_atom(reinterpret_cast<BaseFunctionAtom*>(atom));
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
        case NodeType::LIST_ATOM: {
          return walk_list_atom(reinterpret_cast<ListAtom*>(atom));
        }
        case NodeType::NUMBER_RANGE_ATOM:
          return visitor.visit_number_range_atom(reinterpret_cast<NumberRangeAtom*>(atom));
        default: {
          throw RuntimeException("Invalid atom type:"+type_to_str(atom->node_type_)+std::to_string(atom->node_type_));
        }
      }
    }
};

template<class T> class BaseVisitor {
  public:
    std::vector<continuation_record_t> continuations;

    void visit_specification(AstNode*) {}
    void visit_init(AstNode*) {}
    void visit_body_elements(AstListNode*) {}
    void visit_function_def(FunctionDefNode*, const std::vector<std::pair<T,T>>&) {}
    void visit_derived_def_pre(FunctionDefNode*) {}
    void visit_derived_def(FunctionDefNode*, T) {}
    void visit_rule(RuleNode*) {}
    void visit_statements(AstListNode*) {}
    void visit_statement(AstNode*) {}
    void visit_ifthenelse(IfThenElseNode*, T) {}
    T visit_assert(UnaryNode*, T) { return T(); }
    void visit_seqblock(UnaryNode*) {}
    void visit_seqblock_post() {}
    void visit_parblock(UnaryNode*) {}
    void visit_parblock_post() {}
    T visit_update(UpdateNode*, T, T) { return T(); }
    T visit_update_dumps(UpdateNode *u, T v1, T v2) { return visit_update(u, v1, v2); }
    T visit_call_pre(CallNode*) { return T(); }
    T visit_call_pre(CallNode*, T) { return T(); }
    T visit_call(CallNode*, std::vector<T>&) { return T(); }
    void visit_call_post(CallNode*) {}
    T visit_print(PrintNode*, std::vector<T>&) { return T(); }
    void visit_diedie(DiedieNode*, const T&) {}

    void visit_let(LetNode*, T) {}
    void visit_let_post(LetNode*) {}
    void visit_pop(PopNode*) { }
    void visit_push(PushNode*, T , T)  { }
    void visit_case(CaseNode*, const T, const std::vector<T>&) { }

    void visit_forall_pre(ForallNode*) { }
    void visit_forall_post(ForallNode*) { }

    void visit_iterate(UnaryNode*) { }

    T visit_expression(Expression*, T, T) { return T(); }
    T visit_expression_single(Expression*, T) { return T(); }
    T visit_int_atom(IntAtom*) { return T(); }
    T visit_float_atom(FloatAtom*) { return T(); }
    T visit_rational_atom(RationalAtom*) { return T(); }
    T visit_undef_atom(UndefAtom*) { return T(); }
    T visit_function_atom(FunctionAtom *, const std::vector<T> &) { return T(); }
    T visit_builtin_atom(BuiltinAtom *, const std::vector<T> &) { return T(); }
    void visit_derived_function_atom_pre(FunctionAtom*, const std::vector<T> &) {}
    T visit_derived_function_atom(FunctionAtom*, T) { return T(); }
    T visit_self_atom(SelfAtom*) { return T(); }
    T visit_rule_atom(RuleAtom*) { return T(); }
    T visit_boolean_atom(BooleanAtom*) { return T(); }
    T visit_string_atom(StringAtom*) { return T(); }
    T visit_list_atom(ListAtom*, std::vector<T> &) { return T(); }
    T visit_number_range_atom(NumberRangeAtom*) { return T(); }
};

#endif
