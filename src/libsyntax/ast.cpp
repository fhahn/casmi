#include <map>

#include "libsyntax/ast.h"
#include "libsyntax/driver.h"


static std::map<NodeType, const std::string> node_type_names_ = {
    {NodeType::ASSERT, std::string("ASSERT")},
    {NodeType::UNDEF_ATOM, std::string("UNDEF ATOM")},
    {NodeType::INT_ATOM, std::string("INT ATOM")},
    {NodeType::FLOAT_ATOM, std::string("FLOAT ATOM")},
    {NodeType::SELF_ATOM, std::string("SELF ATOM")},
    {NodeType::RULE_ATOM, std::string("RULE ATOM")},
    {NodeType::BOOLEAN_ATOM, std::string("BOOLEAN ATOM")},
    {NodeType::DUMMY_ATOM, std::string("DUMMY ATOM")},
    {NodeType::INIT, std::string("INIT")},
    {NodeType::IFTHENELSE, std::string("IFTHENELSE")},
    {NodeType::BODY_ELEMENTS, std::string("BODY ELEMENTS")},
    {NodeType::PROVIDER, std::string("PROVIDER")},
    {NodeType::OPTION, std::string("OPTION")},
    {NodeType::ENUM, std::string("ENUM")},
    {NodeType::FUNCTION, std::string("FUNCTION")},
    {NodeType::DERIVED, std::string("DERIVED")},
    {NodeType::RULE, std::string("RULE")},
    {NodeType::EXPRESSION, std::string("EXPRESSION")},
    {NodeType::UPDATE, std::string("UPDATE")},
    {NodeType::SPECIFICATION, std::string("SPECIFICATION")},
    {NodeType::STATEMENT, std::string("STATEMENT")},
    {NodeType::STATEMENTS, std::string("STATEMENTS")},
    {NodeType::STATEMENTS, std::string("SKIP")},
    {NodeType::PARBLOCK, std::string("PARBLOCK")},
    {NodeType::FUNCTION_ATOM, std::string("FUNCTION ATOM")},
    {NodeType::CALL, std::string("CALL NODE")},
};

std::string unknown_type = "unknown node type";

const std::string& type_to_str(NodeType t) {
  try {
    return node_type_names_.at(t);
  } catch (const std::out_of_range& e) {
    return unknown_type;
  }
}

AstNode::AstNode(NodeType node_type) {
    node_type_ = node_type;
    type_ = Type::UNKNOWN;
   // DEBUG(this->to_str());
}

AstNode::AstNode(yy::location& loc, NodeType nt) :
        location(loc), node_type_(nt), type_(Type::UNKNOWN) {}

AstNode::AstNode(yy::location& loc, NodeType nt, Type t) :
        location(loc), node_type_(nt), type_(t) {}

AstNode::~AstNode() {
  // no dynamically alloceted stuff here
}

std::string AstNode::to_str() {
    return std::string("AStNode: ") + type_to_str(node_type_);
}

bool AstNode::equals(AstNode *other) {
  return node_type_ == other->node_type_;
}

std::string AstNode::location_str() const {
  if (location.begin.filename != nullptr) {
    return *location.begin.filename;
  } else {
    return "NO FILE";
  }
}

AstListNode::AstListNode(yy::location& loc, NodeType node_type) :
        AstNode(loc, node_type) {}

AstListNode::~AstListNode() {
  for (auto n : nodes) {
    delete n;
  }
  nodes.clear();
}

void AstListNode::add(AstNode* n) {
    this->nodes.push_back(n);
}

bool AstListNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  AstListNode *other_cast = static_cast<AstListNode*>(other);

  auto ast1_iter = nodes.begin();
  auto ast2_iter = other_cast->nodes.begin();
  
  while (ast1_iter < nodes.end() && ast2_iter < other_cast->nodes.end()) {
    if (!((*ast1_iter)->equals(*ast2_iter))) {
      return false;
    }
    ast1_iter += 1;
    ast2_iter += 1;
  }

  if (ast1_iter == nodes.cend() && ast2_iter == other_cast->nodes.cend()) {
    return true;
  } else {
    return false;
  }
}

FunctionDefNode::FunctionDefNode(yy::location& loc, Symbol* sym) 
    : AstNode(loc, NodeType::FUNCTION), sym(sym) {}

FunctionDefNode::~FunctionDefNode() {
  // sym is deleted in the symbol table
}


IfThenElseNode::IfThenElseNode(yy::location& loc, ExpressionBase *condition, AstNode *then, AstNode *els)
    : AstNode(loc, NodeType::IFTHENELSE), condition_(condition), then_(then), else_(els) {}


IntAtom::IntAtom(yy::location& loc, INT_T val) :
        AtomNode(loc, NodeType::INT_ATOM, Type::INT) {
  val_ = val;
}

IntAtom::~IntAtom() {}

bool IntAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  IntAtom *other_cast = static_cast<IntAtom*>(other);
  return val_ == other_cast->val_;
}

FloatAtom::FloatAtom(yy::location& loc, FLOAT_T val) :
        AtomNode(loc, NodeType::FLOAT_ATOM, Type::FLOAT) {
  val_ = val;
}

FloatAtom::~FloatAtom() {}

bool FloatAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  FloatAtom *other_cast = static_cast<FloatAtom*>(other);
  return val_ == other_cast->val_;
}


UndefAtom::UndefAtom(yy::location& loc) :
        AtomNode(loc, NodeType::UNDEF_ATOM, Type::UNKNOWN) {}

bool UndefAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  } else {
    return true;
  }
}


SelfAtom::SelfAtom(yy::location& loc) :
        AtomNode(loc, NodeType::SELF_ATOM, Type::SELF) { DEBUG("TRUE");}

bool SelfAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  } else {
    return true;
  }
}


BooleanAtom::BooleanAtom(yy::location& loc, bool value) :
        AtomNode(loc, NodeType::BOOLEAN_ATOM, Type::BOOLEAN), value(value) {}

bool BooleanAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  BooleanAtom *other_b = reinterpret_cast<BooleanAtom*>(other);
  return value == other_b->value;
}



RuleAtom::RuleAtom(yy::location& loc, const std::string& name) :
        AtomNode(loc, NodeType::RULE_ATOM, Type::RULEREF), name(name) {}

RuleAtom::~RuleAtom() {}

bool RuleAtom::equals(AstNode *other) {
  throw "NOT IMPLEMENTED";
}


FunctionAtom::FunctionAtom(yy::location& loc, const std::string name) 
    : FunctionAtom(loc, name, nullptr) {
}

FunctionAtom::FunctionAtom(yy::location& loc, const std::string name,
                           std::vector<ExpressionBase*> *args) 
    : AtomNode(loc, NodeType::FUNCTION_ATOM, Type::UNKNOWN), symbol(nullptr),
      name(name), arguments(args) {
      
}

FunctionAtom::~FunctionAtom() {
}

bool FunctionAtom::equals(AstNode *other) {
  UNUSED(other);
  throw "not implemented";
    /*
  if (!AstNode::equals(other)) {
    return false;
  }

  IntAtom *other_cast = static_cast<IntAtom*>(other);
  return val_ == other_cast->val_;
  */
}

Expression::Expression(yy::location& loc, ExpressionBase *left, ExpressionBase *right,
                       Expression::Operation op)
                       : ExpressionBase(loc, NodeType::EXPRESSION, Type::UNKNOWN),
                         left_(left), right_(right), op(op) {
  // Propagate known types
  /*
  if (left_ == nullptr) {
    if(right_->type_ != Type::UNKNOWN) {
      type_ = right_->type_;
    }
  } else if (right_ == nullptr) {
    if (left_->type_ != Type::UNKNOWN) {
      type_ = left_->type_;
    }
  } else {
    if (left_->type_ == right_->type_) {
      type_ = right_->type_;
    }
  }
  */
}

Expression::~Expression() {
  delete left_;
  delete right_;
}

bool Expression::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  Expression *other_cast = static_cast<Expression*>(other);
  if (left_ == nullptr || other_cast->left_ == nullptr) {
    return left_ == nullptr && 
           other_cast->left_ == nullptr &&
           right_->equals(other_cast->right_) &&
           op == other_cast->op;
  } else {
    return left_->equals(other_cast->left_) &&
           right_->equals(other_cast->right_) &&
           op == other_cast->op;
  }
}

std::string operator_to_str(const Expression::Operation op) {
  switch(op) {
    case Expression::Operation::ADD: return "+";
    case Expression::Operation::SUB: return "-";
    case Expression::Operation::MUL: return "*";
    case Expression::Operation::DIV: return "/";
    case Expression::Operation::MOD: return "%";
    case Expression::Operation::RAT_DIV: return "div";

    case Expression::Operation::EQ: return "=";
    case Expression::Operation::NEQ: return "!=";

    case Expression::Operation::LESSER: return "<";
    case Expression::Operation::GREATER: return ">";
    case Expression::Operation::LESSEREQ: return "<=";
    case Expression::Operation::GREATEREQ: return ">=";
    default: return "unknown";
  }
}

UpdateNode::UpdateNode(yy::location& loc, FunctionAtom *func, ExpressionBase *expr)
    : AstNode(loc, NodeType::UPDATE), func(func), expr_(expr) {
}

UpdateNode::~UpdateNode() {
  delete func;
  delete expr_;
}

bool UpdateNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  UpdateNode *other_cast = static_cast<UpdateNode*>(other);
  return expr_->equals(other_cast->expr_) && func->equals(other_cast->func);
}

UnaryNode::UnaryNode(yy::location& loc, NodeType node_type, AstNode *child) : AstNode(loc, node_type) {
  child_ = child;
}

UnaryNode::~UnaryNode() {
  delete child_;
}

bool UnaryNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  UnaryNode *other_cast = static_cast<UnaryNode*>(other);
  return child_->equals(other_cast->child_);
}

RuleNode::RuleNode(yy::location& loc, AstNode *child, const std::string& n)
  : RuleNode(loc, child, n, nullptr) {}

RuleNode::RuleNode(yy::location& loc, AstNode *child, const std::string &name,
                   const std::vector<Type> *args) 
  : UnaryNode(loc, NodeType::RULE, child), name(name), arguments(args) {}

CallNode::CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref)
    : CallNode(loc, rule_name, ruleref, nullptr) {}

CallNode::CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref,
                   std::vector<ExpressionBase*> *args)
    : AstNode(loc, NodeType::CALL, Type::NO_TYPE), rule_name(rule_name),
      direct(direct), rule(nullptr), arguments(args), ruleref(ruleref) {}
