#include "libparse/ast.h"
#include "libparse/driver.h"

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

Type AstNode::propagate_types(Type top, casmi_driver &driver) {
  return Type::NO_TYPE;
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

Type AstListNode::propagate_types(Type top, casmi_driver &driver) {
  for (auto n : nodes) {
    n->propagate_types(top, driver);
  }
  return Type::NO_TYPE;
}


Type AtomNode::propagate_types(Type top, casmi_driver &driver) {
  return Type::UNKNOWN;
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


Type IntAtom::propagate_types(Type top, casmi_driver &driver) {
  return Type::INT;
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

Type UndefAtom::propagate_types(Type top, casmi_driver &driver) {
  return top;
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


Type FunctionAtom::propagate_types(Type top, casmi_driver &driver) {
  return driver.current_symbol_table->get(func_);
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


Type Expression::propagate_types(Type top, casmi_driver &driver) {
  if (left_ != nullptr) {
    Type down_t = left_->propagate_types(top, driver);
    if (down_t != right_->propagate_types(top, driver)) {
      driver.error(location, "type of expressions did not match");
    }
    return down_t;
  } else {
    return right_->propagate_types(top, driver);
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

Type UpdateNode::propagate_types(Type top, casmi_driver &driver) {
  Type sym_type = driver.current_symbol_table->get(sym_);
  if (sym_type == Type::INVALID) {
    driver.error(sym_->location, "use of undefined function `"+sym_->name_+"`");
  }
  if (sym_type != expr_->propagate_types(sym_type, driver)) {
    driver.error(location, "type of `"+sym_->name_+
                            "` does not match type of expression");
  }
  return sym_type;
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

Type UnaryNode::propagate_types(Type top, casmi_driver &driver) {
  return child_->propagate_types(top, driver);
}

AtomNode* create_atom(yy::location& loc, SymbolUsage *val) {
    return new FunctionAtom(loc, val);
}

AtomNode* create_atom(yy::location& loc, INT_T val) {
    return new IntAtom(loc, val);
}

AtomNode* create_atom(yy::location& loc) {
    return new UndefAtom(loc);
}
