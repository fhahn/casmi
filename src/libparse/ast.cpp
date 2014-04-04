#include <map>

#include "libparse/ast.h"
#include "libparse/driver.h"


static std::map<NodeType, const std::string> node_type_names_ = {
    {NodeType::UNDEF_ATOM, std::string("UNDEF ATOM")},
    {NodeType::INT_ATOM, std::string("INT ATOM")},
    {NodeType::DUMMY_ATOM, std::string("DUMMY ATOM")},
    {NodeType::INIT, std::string("INIT")},
    {NodeType::BODY_ELEMENTS, std::string("BODY ELEMENTS")},
    {NodeType::PROVIDER, std::string("PROVIDER")},
    {NodeType::OPTION, std::string("OPTION")},
    {NodeType::ENUM, std::string("ENUM")},
    {NodeType::FUNCTION, std::string("FUNCTION")},
    {NodeType::DERIVED, std::string("DERIVED")},
    {NodeType::RULE, std::string("RULE")},
    {NodeType::SPECIFICATION, std::string("SPECIFICATION")},
    {NodeType::EXPRESSION, std::string("EXPRESSION")},
    {NodeType::UPDATE, std::string("UPDATE")},
    {NodeType::STATEMENT, std::string("STATEMENT")},
    {NodeType::PARBLOCK, std::string("PARBLOCK")},
    {NodeType::STATEMENTS, std::string("STATEMENTS")},
    {NodeType::FUNCTION_ATOM, std::string("FUNCTION ATOM")},
};

const std::string& type_to_str(NodeType t) {
    return node_type_names_.at(t);
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
    return std::string("AStNode: ")+ node_type_names[node_type_];
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

UndefAtom::UndefAtom(yy::location& loc) :
        AtomNode(loc, NodeType::UNDEF_ATOM, Type::UNKNOWN) {}

bool UndefAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  } else {
    return true;
  }
}

FunctionAtom::FunctionAtom(yy::location& loc, SymbolUsage *val) :
        AtomNode(loc, NodeType::FUNCTION_ATOM, Type::UNKNOWN), func_(val) {}

FunctionAtom::~FunctionAtom() {
  delete func_;
}

bool FunctionAtom::equals(AstNode *other) {
  throw "not implemented";
    /*
  if (!AstNode::equals(other)) {
    return false;
  }

  IntAtom *other_cast = static_cast<IntAtom*>(other);
  return val_ == other_cast->val_;
  */
}

Expression::Expression(yy::location& loc, Expression *left, AtomNode *right) : AstNode(loc, NodeType::EXPRESSION) {
  left_ = left;
  right_ = right;

  // Propagate known types
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
    return left_ == nullptr && other_cast->left_ == nullptr && right_->equals(other_cast->right_);
  } else {
    return left_->equals(other_cast->left_) && right_->equals(other_cast->right_);
  }
}

UpdateNode::UpdateNode(yy::location& loc, SymbolUsage *sym, Expression *expr) : AstNode(loc, NodeType::UPDATE),
                                           sym_(sym), expr_(expr) {
}

UpdateNode::~UpdateNode() {
  delete sym_;
  delete expr_;
}

bool UpdateNode::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  UpdateNode *other_cast = static_cast<UpdateNode*>(other);
  return expr_->equals(other_cast->expr_) && sym_->equals(other_cast->sym_);
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
  : UnaryNode(loc, NodeType::RULE, child), name(n) {}


AtomNode* create_atom(yy::location& loc, SymbolUsage *val) {
    return new FunctionAtom(loc, val);
}

AtomNode* create_atom(yy::location& loc, INT_T val) {
    return new IntAtom(loc, val);
}

AtomNode* create_atom(yy::location& loc) {
    return new UndefAtom(loc);
}
