#include <map>

#include "libsyntax/ast.h"
#include "libsyntax/driver.h"


static std::map<NodeType, const std::string> node_type_names_ = {
    {NodeType::ASSERT, std::string("ASSERT")},
    {NodeType::UNDEF_ATOM, std::string("UNDEF ATOM")},
    {NodeType::INT_ATOM, std::string("INT ATOM")},
    {NodeType::FLOAT_ATOM, std::string("FLOAT ATOM")},
    {NodeType::SELF_ATOM, std::string("SELF ATOM")},
    {NodeType::STRING_ATOM, std::string("STRING ATOM")},
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
    {NodeType::SKIP, std::string("SKIP")},
    {NodeType::PARBLOCK, std::string("PARBLOCK")},
    {NodeType::SEQBLOCK, std::string("SEQBLOCK")},
    {NodeType::FUNCTION_ATOM, std::string("FUNCTION ATOM")},
    {NodeType::CALL, std::string("CALL NODE")},
    {NodeType::PRINT, std::string("PRINT NODE")},
};

std::string unknown_type = "unknown node type";

const std::string& type_to_str(NodeType t) {
  try {
    return node_type_names_.at(t);
  } catch (const std::out_of_range& e) {
    return unknown_type;
  }
}

AstNode::AstNode(NodeType node_type) : type_(TypeType::UNKNOWN){
    node_type_ = node_type;
   // DEBUG(this->to_str());
}

AstNode::AstNode(yy::location& loc, NodeType nt) :
        location(loc), node_type_(nt), type_(TypeType::UNKNOWN) {}

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

FunctionDefNode::FunctionDefNode(yy::location& loc, Function* sym) 
    : AstNode(loc, NodeType::FUNCTION), sym(sym) {
}

FunctionDefNode::~FunctionDefNode() {
  // sym is deleted in the symbol table
}


IfThenElseNode::IfThenElseNode(yy::location& loc, ExpressionBase *condition, AstNode *then, AstNode *els)
    : AstNode(loc, NodeType::IFTHENELSE), condition_(condition), then_(then), else_(els) {}


IntAtom::IntAtom(yy::location& loc, INT_T val) :
        AtomNode(loc, NodeType::INT_ATOM, Type(TypeType::INT)) {
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
        AtomNode(loc, NodeType::FLOAT_ATOM, Type(TypeType::FLOAT)) {
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
        AtomNode(loc, NodeType::UNDEF_ATOM, Type(TypeType::UNKNOWN)) {}

bool UndefAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  } else {
    return true;
  }
}


SelfAtom::SelfAtom(yy::location& loc) :
        AtomNode(loc, NodeType::SELF_ATOM, Type(TypeType::SELF)) { DEBUG("TRUE");}

bool SelfAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  } else {
    return true;
  }
}


BooleanAtom::BooleanAtom(yy::location& loc, bool value) :
        AtomNode(loc, NodeType::BOOLEAN_ATOM, Type(TypeType::BOOLEAN)), value(value) {}

bool BooleanAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  BooleanAtom *other_b = reinterpret_cast<BooleanAtom*>(other);
  return value == other_b->value;
}



RuleAtom::RuleAtom(yy::location& loc, const std::string&& name) :
        AtomNode(loc, NodeType::RULE_ATOM, Type(TypeType::RULEREF)), name(name) {}

RuleAtom::~RuleAtom() {}

bool RuleAtom::equals(AstNode *other) {
  throw "NOT IMPLEMENTED";
}


StringAtom::StringAtom(yy::location& loc, std::string&& string) :
        AtomNode(loc, NodeType::STRING_ATOM, Type(TypeType::STRING)), string(string) {
  DEBUG("StringAtom "<<string);
}

StringAtom::~StringAtom() {}

bool StringAtom::equals(AstNode *other) {
  throw "NOT IMPLEMENTED";
}

BaseFunctionAtom::BaseFunctionAtom(yy::location& loc, NodeType t, const std::string name,
                           std::vector<ExpressionBase*> *args) 
    : AtomNode(loc, t, Type(TypeType::UNKNOWN)),
       name(name), arguments(args) {
}


FunctionAtom::FunctionAtom(yy::location& loc, const std::string name)
    : FunctionAtom(loc, name, nullptr) {
}

FunctionAtom::FunctionAtom(yy::location& loc, const std::string name,
                           std::vector<ExpressionBase*> *args) 
    : BaseFunctionAtom(loc, NodeType::FUNCTION_ATOM, name, args), symbol_type(SymbolType::UNSET) {
}

FunctionAtom::~FunctionAtom() {
}

bool FunctionAtom::equals(AstNode *other) {
  if (!AstNode::equals(other)) {
    return false;
  }

  FunctionAtom *other_func = reinterpret_cast<FunctionAtom*>(other);

  if (symbol_type == SymbolType::UNSET) {
    return name == other_func->name;
  }

  if (arguments && other_func->arguments &&
      arguments->size() == other_func->arguments->size()) {
    for (size_t i=0; i < arguments->size(); i++) {
      if (!arguments->at(i)->equals(other_func->arguments->at(i))) {
        return false;
      }
    }
  }

  if (! (!arguments && !other_func->arguments)) {
    return false;
  }

  if (symbol_type == FunctionAtom::SymbolType::FUNCTION && symbol_type == other_func->symbol_type) {
    if (symbol && other_func->symbol &&
        symbol->name() != other_func->symbol->name()) {
      return false;
    }

    if (! (!symbol && !other_func->symbol)) {
      return false;
    }
    return name == other_func->name;
  } else if (symbol_type == FunctionAtom::SymbolType::PARAMETER && symbol_type == other_func->symbol_type) {
    return name == other_func->name && offset == other_func->offset;
  }
  return false;
}

BuiltinAtom::BuiltinAtom(yy::location& loc, const std::string name,
                           std::vector<ExpressionBase*> *args) 
    : BaseFunctionAtom(loc, NodeType::BUILTIN_ATOM, name, args), return_type(nullptr) {
  if (name == "pow") {
    Type *a1 = new Type(TypeType::UNKNOWN);
    Type *a2 = new Type(TypeType::UNKNOWN);
    return_type = new Type(TypeType::UNKNOWN);

    a1->unify(a2);
    a2->unify(return_type);
    types = { a1, a2 };
  } else if (name == "hex") {
    Type *a1 = new Type(TypeType::INT);
    return_type = new Type(TypeType::STRING);

    types = { a1 };
  } else if (name == "nth") {
    Type *a1 = new Type(TypeType::TUPLE_OR_LIST, new Type(TypeType::UNKNOWN));
    Type *a2 = new Type(TypeType::INT);
    return_type = new Type(TypeType::UNKNOWN);

    a1->internal_type->unify(return_type);
    types = { a1, a2 };
  } else if (name == "cons"){
    Type *a1 = new Type(TypeType::UNKNOWN);
    Type *a2 = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));
    return_type = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));

    a1->unify(a2->internal_type);
    a2->unify(return_type);
    types = { a1, a2 };
  } else if (name == "len") {
    Type *a1 = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));
    return_type = new Type(TypeType::INT);

    types = { a1 };
  } else if (name == "tail") {
    Type *a1 = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));
    return_type = new Type(TypeType::UNKNOWN);
    a1->unify(return_type);

    types = { a1 };
  } else if (name == "peek") {
    Type *a1 = new Type(TypeType::LIST, new Type(TypeType::UNKNOWN));
    return_type = new Type(TypeType::UNKNOWN);
    a1->internal_type->unify(return_type);

    types = { a1 };
 } else {
    assert(0);
  }
}

BuiltinAtom::~BuiltinAtom() {}

bool BuiltinAtom::equals(AstNode *other) {
  throw "BuiltinAtom::equals() not implemented";
}

ListAtom::ListAtom(yy::location& loc, std::vector<ExpressionBase*> *exprs)
    : AtomNode(loc, NodeType::LIST_ATOM, TypeType::UNKNOWN), expr_list(exprs) , tmp_list() {
  // TODO LEAK!
  type_ = Type(TypeType::LIST, new Type(TypeType::UNKNOWN));
}

NumberRangeAtom::NumberRangeAtom(yy::location& loc, IntAtom *start, IntAtom *end) :
    AtomNode(loc, NodeType::NUMBER_RANGE_ATOM, TypeType::UNKNOWN), perm_list() {
  type_ = Type(TypeType::LIST, new Type(TypeType::INT));
  INT_T i_start = start->val_;
  INT_T i_end = end->val_;

  if (i_start <= i_end) {
    for (INT_T i=i_start; i <= i_end; i++) {
      perm_list.values.push_back(Value(i));
    }
  }
}
Expression::Expression(yy::location& loc, ExpressionBase *left, ExpressionBase *right,
                       Expression::Operation op)
                       : ExpressionBase(loc, NodeType::EXPRESSION, Type(TypeType::UNKNOWN)),
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


RuleNode::RuleNode(yy::location& loc, AstNode *child, const std::string& name)
  :  UnaryNode(loc, NodeType::RULE, child), name(std::move(name)), arguments(), binding_offsets() {}

RuleNode::RuleNode(yy::location& loc,
                   AstNode *child,
                   const std::string &name,
                   std::vector<Type*>& args)
  : UnaryNode(loc, NodeType::RULE, child), name(std::move(name)),
    arguments(std::move(args)), binding_offsets(std::move(binding_offsets)) {}


CallNode::CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref)
    : CallNode(loc, rule_name, ruleref, nullptr) {}

CallNode::CallNode(yy::location& loc, const std::string& rule_name, ExpressionBase *ruleref,
                   std::vector<ExpressionBase*> *args)
    : AstNode(loc, NodeType::CALL, Type(TypeType::NO_TYPE)), rule_name(rule_name),
      rule(nullptr), arguments(args), ruleref(ruleref) {}


PrintNode::PrintNode(yy::location& loc, const std::vector<ExpressionBase*> &atoms)
    : AstNode(loc, NodeType::PRINT, Type(TypeType::NO_TYPE)), atoms(std::move(atoms)){

}

LetNode::LetNode(yy::location& loc, Type type, const std::string& identifier,
            ExpressionBase *expr, AstNode *stmt) 
    : AstNode(loc, NodeType::LET, type), identifier(identifier), expr(expr), stmt(stmt) {}

