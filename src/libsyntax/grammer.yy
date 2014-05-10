%skeleton "lalr1.cc" /* -*- C++ -*- */
%defines
%define parser_class_name {casmi_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert


%code requires
{
    #include <cstdint>
    #include <string>
    #include <utility>

    #include "libsyntax/ast.h"
    #include "libsyntax/types.h"
    class Driver;
}

// The parsing context.
%parse-param { Driver& driver }
%lex-param   { Driver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  // Error messages are printed in Driver, I guess location does not 
  // need to know about the filename
  //@$.begin.filename = @$.end.filename = &driver.filename_;
};

%define parse.trace
%define parse.error verbose

%code {
    #include "libsyntax/driver.h"

    std::pair<bool, bool> parse_function_attributes(Driver& driver, const yy::location& loc,
                                                    const std::vector<std::string>& attribute_names) {
        bool is_static = false;
        bool is_symbolic = false;
        bool is_controlled = false;

        for (const auto& attribute_name : attribute_names) {
            if (attribute_name == "static") {
                if (is_static) {
                    driver.error(loc, "`static` attribute can only be used once per function");
                    break;
                } else {
                    is_static = true;
                }
            } else if (attribute_name == "symbolic") {
                if (is_symbolic) {
                    driver.error(loc, "`symbolic` attribute can only be used once per function");
                    break;
                } else {
                    is_symbolic = true;
                }
            } else if (attribute_name == "controlled") {
                if (is_controlled) {
                    driver.error(loc, "`controlled` attribute can only be used once per function");
                    break;
                } else {
                    is_controlled = true;
                }
            } else {
              driver.error(loc, "`"+attribute_name+"` is no valid function attribute, only static, symbolic and controlled are allowed");
            }
        }
        if (is_static && is_controlled) {
            driver.error(loc, "attributes `controlled` and `static` are mutually exclusive");
        }

        return std::pair<bool, bool>(is_static, is_symbolic);
    }
}

%token AND OR XOR NOT ASSERT ASSURE DIEDIE IMPOSSIBLE SKIP SEQBLOCK ENDSEQBLOCK
%token PARBLOCK ENDPARBLOCK LET IN IF THEN ELSE PRINT DEBUGINFO DUMPS PUSH INTO
%token POP FROM FORALL ITERATE DO CALL CASE DEFAULT OF ENDCASE INITIALLY FUNCTION
%token DERIVED ENUM RULE PROVIDER INIT OPTION SELF UNDEF TRUE FALSE CASM SYMBOL
%token INTERN RATIONAL_DIV OBJDUMP

%token DOTDOT ARROW UPDATE NEQUAL LESSEQ GREATEREQ SEQBLOCK_BRACKET ENDSEQBLOCK_BRACKET

%token
    END  0  "end of file"
    PLUS    "+"
    MINUS   "-"
    EQ      "="
    LPAREN  "("
    RPAREN  ")"
    LSQPAREN  "["
    RSQPAREN  "]"
    LCURPAREN  "{"
    RCURPAREN  "}"
    DOT "."
    COLON ":"
    AT "@"
    COMMA ","
    LESSER "<"
    GREATER ">"
    STAR    "*"
    SLASH   "/"
    PERCENT "%"
    ;
%token FLOATCONST INTCONST RATIONALCONST STRCONST
%token <std::string> IDENTIFIER "identifier"

%type <AstNode*> INIT_SYNTAX BODY_ELEMENT SPECIFICATION RULE_SYNTAX STATEMENT
%type <UnaryNode*> PARBLOCK_SYNTAX KW_PARBLOCK_SYNTAX SEQBLOCK_SYNTAX
%type <UnaryNode*> ASSERT_SYNTAX KW_SEQBLOCK_SYNTAX ITERATE_SYNTAX
%type <AstListNode*> BODY_ELEMENTS STATEMENTS
%type <AtomNode*> NUMBER VALUE NUMBER_RANGE
%type <std::pair<ExpressionBase*, ExpressionBase*>> INITIALIZER
%type <std::vector<std::pair<ExpressionBase*, ExpressionBase*>>*> INITIALIZER_LIST INITIALIZERS
%type <ExpressionBase*> EXPRESSION BRACKET_EXPRESSION ATOM
%type <std::vector<ExpressionBase*>*> EXPRESSION_LIST EXPRESSION_LIST_NO_COMMA LISTCONST
%type <UpdateNode*> UPDATE_SYNTAX
%type <INT_T> INTCONST
%type <FLOAT_T> FLOATCONST
%type <std::string> STRCONST
%type <Function*> FUNCTION_DEFINITION DERIVED_SYNTAX
%type <BaseFunctionAtom*> FUNCTION_SYNTAX 
%type <std::pair<std::vector<Type*>, Type*>> FUNCTION_SIGNATURE
%type <Type*> TYPE_SYNTAX
%type <Type*> PARAM
%type <std::vector<Type*>> PARAM_LIST_NO_COMMA PARAM_LIST
%type <std::vector<Type*>> TYPE_IDENTIFIER_STARLIST
%type <std::string> RULEREF
%type <IfThenElseNode*> IFTHENELSE
%type <CallNode*> CALL_SYNTAX
%type <std::vector<ExpressionBase*>> DEBUG_ATOM_LIST
%type <PrintNode*> PRINT_SYNTAX DEBUGINFO_SYNTAX
%type <LetNode*> LET_SYNTAX
%type<std::vector<Type*>> TYPE_SYNTAX_LIST
%type <PushNode*> PUSH_SYNTAX
%type <PopNode*> POP_SYNTAX
%type <std::pair<AtomNode*, AstNode*>> CASE_LABEL_STRING CASE_LABEL_NUMBER CASE_LABEL_DEFAULT CASE_LABEL_IDENT CASE_LABEL
%type <std::vector<std::pair<AtomNode*, AstNode*>>> CASE_LABEL_LIST
%type <CaseNode*> CASE_SYNTAX
%type <ForallNode*> FORALL_SYNTAX
%type <std::vector<std::string>> IDENTIFIER_LIST IDENTIFIER_LIST_NO_COMMA
%type <Enum*> ENUM_SYNTAX;
%type <std::vector<std::pair<std::string, std::vector<std::string>>>> DUMPSPEC_LIST
%type <std::pair<std::string, std::vector<std::string>>> DUMPSPEC


%start SPECIFICATION

/* TODO: Check! */
%right UMINUS 
%right UPLUS
%left XIF

%precedence THEN ELSE

%precedence UPDATE PRINT ASSURE ASSERT DIEDIE NOT

%nonassoc ","
%nonassoc FLOATCONST INTCONST STRCONST RATIONALCONST IDENTIFIER
%nonassoc AND OR
%nonassoc "=" "<" ">"  NEQUAL LESSEQ GREATEREQ
%left "-" "+" XOR
%left RATIONAL_DIV "*" "/" "%"

%%


SPECIFICATION: HEADER BODY_ELEMENTS { driver.result = $2; } /* TODO: header ignored atm */ 
             | BODY_ELEMENTS { driver.result = $1; }
             ;

HEADER: CASM IDENTIFIER
      ;

BODY_ELEMENTS: BODY_ELEMENTS BODY_ELEMENT { $1->add($2); $$ = $1; }
            | BODY_ELEMENT { $$ = new AstListNode(@$, NodeType::BODY_ELEMENTS); $$->add($1); }
            ;

BODY_ELEMENT: PROVIDER_SYNTAX { $$ = new AstNode(NodeType::PROVIDER); }
           | OPTION_SYNTAX { $$ = new AstNode(NodeType::OPTION); }
           | ENUM_SYNTAX { $$ = new EnumDefNode(@$, $1); }
           | FUNCTION_DEFINITION {
                $$ = new FunctionDefNode(@$, $1);
                if ($1->is_builtin()) {
                    driver.error(@$, "cannot use `"+$1->name+"` as function identifier because it is a builtin name");
                }
                if (!driver.function_table.add($1)) {
                    driver.error(@$, "redefinition of symbol");
                    // if another symbol with same name exists we need to delete
                    // the symbol here, because it is not inserted in the symbol table
                    delete $1;
                }
            }
           | DERIVED_SYNTAX {
                $1->binding_offsets = std::move(driver.binding_offsets);
                driver.binding_offsets.clear();
                $$ = new FunctionDefNode(@$, $1);
                if (!driver.function_table.add($1)) {
                    driver.error(@$, "redefinition of symbol");
                    // if another symbol with same name exists we need to delete
                    // the symbol here, because it is not inserted in the symbol table
                    delete $1;
                }
            }
           | INIT_SYNTAX { $$ = $1; }
           | RULE_SYNTAX { $$ = $1;
              // TODO check, we trust bison to pass only RuleNodes up
              if (!driver.add(reinterpret_cast<RuleNode*>($1))) {
                    driver.error(@$, "redefinition of rule");
                    // we do not need to delete $1 here, because it's already in
                    // the AST, so it will be deleted later
              } 
           }
           ;

INIT_SYNTAX: INIT IDENTIFIER { $$ = new AstNode(NodeType::INIT); driver.init_name = $2; }
           ;

PROVIDER_SYNTAX: PROVIDER IDENTIFIER 
          ;

OPTION_SYNTAX: OPTION IDENTIFIER "." IDENTIFIER IDENTIFIER;

ENUM_SYNTAX: ENUM IDENTIFIER "=" "{" IDENTIFIER_LIST "}" {
                $$ = new Enum($2);
                if (!driver.function_table.add($$)) {
                    driver.error(@$, "redefinition of symbol `"+$2+"`");
                }
                for (const std::string& name : $5) {
                    if ($$->add_enum_element(name)) {
                        if (!driver.function_table.add_enum_element(name, $$)) {
                            driver.error(@$, "redefinition of symbol `"+name+"`");
                        }
                    } else {
                        driver.error(@$, "name `"+name+"` already used in enum");
                    }
                }
           }
           ;

DERIVED_SYNTAX: DERIVED IDENTIFIER "(" PARAM_LIST ")" "=" EXPRESSION {
                  // TODO: 2nd argument should be a reference
                  $$ = new Function($2, $4, $7, new Type(TypeType::UNKNOWN));
                }
              | DERIVED IDENTIFIER "=" EXPRESSION {
                  $$ = new Function($2, $4, new Type(TypeType::UNKNOWN));
                }
              | DERIVED IDENTIFIER "(" ")" "=" EXPRESSION {
                  $$ = new Function($2, $6, new Type(TypeType::UNKNOWN));
                }
              /* again with type syntax */
              | DERIVED IDENTIFIER "(" PARAM_LIST ")" ":" TYPE_SYNTAX "=" EXPRESSION {
                  $$ = new Function($2, $4, $9, $7);
                }
              | DERIVED IDENTIFIER ":" TYPE_SYNTAX "=" EXPRESSION {
                  $$ = new Function($2, $6, $4);
                }
              | DERIVED IDENTIFIER "(" ")" ":" TYPE_SYNTAX "=" EXPRESSION {
                  $$ = new Function($2, $8, $6);
                }
              ;

FUNCTION_DEFINITION: FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS {
                        auto attrs = parse_function_attributes(driver, @$, $3);
                        $$ = new Function(attrs.first, attrs.second, $5, $6.first, $6.second, $7);
                    }
           | FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE {
                        auto attrs = parse_function_attributes(driver, @$, $3);
                        $$ = new Function(attrs.first, attrs.second, $5, $6.first, $6.second, nullptr);
                    }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
                   { $$ = new Function($2, $3.first, $3.second, $4); }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE
                   { $$ = new Function($2, $3.first, $3.second, nullptr); }
           ;


IDENTIFIER_LIST: IDENTIFIER_LIST_NO_COMMA "," { $$ = std::move($1); }
               | IDENTIFIER_LIST_NO_COMMA { $$ = std::move($1); }
               ;

IDENTIFIER_LIST_NO_COMMA: IDENTIFIER_LIST_NO_COMMA "," IDENTIFIER { $$ = std::move($1); $$.push_back($3); }
                        | IDENTIFIER { $$ = std::vector<std::string>(); $$.push_back($1); }

FUNCTION_SIGNATURE: ":" ARROW TYPE_SYNTAX 
                  /* this constructor is implementation dependant! */
                  { 
                    std::vector<Type*> foo;
                    $$ = std::pair<std::vector<Type*>, Type*>(foo, $3); }
                  | ":" TYPE_IDENTIFIER_STARLIST ARROW TYPE_SYNTAX
                  { $$ = std::pair<std::vector<Type*>, Type*>($2, $4); }
                  ;

PARAM: IDENTIFIER ":" TYPE_SYNTAX {
        size_t size = driver.binding_offsets.size();
        driver.binding_offsets[$1] = size;
        $$ = $3;
     }
     | IDENTIFIER {
        size_t size = driver.binding_offsets.size();
        driver.binding_offsets[$1] = size;
        // TODO: fail for rules without types and print warnings
        $$ = new Type(TypeType::INT);
     }
     ;


PARAM_LIST: PARAM_LIST_NO_COMMA { $$ = std::move($1); }
          | PARAM_LIST_NO_COMMA "," { $$ = std::move($1); }

PARAM_LIST_NO_COMMA: PARAM_LIST_NO_COMMA "," PARAM
                   {
                        $$ = std::move($1);
                        $$.push_back($3);
                   }
                   | PARAM
                   { 
                        $$.push_back($1);
                   }
                   ;


TYPE_IDENTIFIER_STARLIST: TYPE_SYNTAX "*" TYPE_IDENTIFIER_STARLIST 
                        {
                            $3.insert($3.begin(), $1);
                            $$ = std::move($3);
                        }
                        | TYPE_SYNTAX "*" 
                        { // TODO: limit memory size
                            $$.push_back($1);
                        }
                        | TYPE_SYNTAX 
                        { 
                            $$.push_back($1);
                        }
                        ;

TYPE_SYNTAX: IDENTIFIER { $$ = new Type($1); /* TODO check invalid types */}
               | IDENTIFIER "(" TYPE_SYNTAX_LIST ")" {
                $$ = new Type($1, $3);
               }
               | IDENTIFIER "(" NUMBER DOTDOT NUMBER ")"
               ;

TYPE_SYNTAX_LIST: TYPE_SYNTAX "," TYPE_SYNTAX_LIST {
                      $3.push_back($1);
                      $$ = std::move($3);
                    }
                    | TYPE_SYNTAX "," { $$.push_back($1); }
                    | TYPE_SYNTAX { $$.push_back($1); }
                    ;

INITIALIZERS: INITIALLY "{" INITIALIZER_LIST "}" { $$ = $3; }
            | INITIALLY "{" "}" { $$ = nullptr; }
            ;

INITIALIZER_LIST: INITIALIZER_LIST "," INITIALIZER { $$ = $1; $1->push_back($3); }
                | INITIALIZER_LIST "," { $$ = $1; }
                | INITIALIZER { $$ = new std::vector<std::pair<ExpressionBase*, ExpressionBase*>>(); $$->push_back($1); }
                ;


INITIALIZER: ATOM { $$ = std::pair<ExpressionBase*, ExpressionBase*>(nullptr, $1); }
           | ATOM ARROW ATOM { $$ = std::pair<ExpressionBase*, ExpressionBase*>($1, $3); }
           ;

ATOM: FUNCTION_SYNTAX { $$ = $1; }
    | VALUE { $$ = $1; }
    | BRACKET_EXPRESSION { $$ = $1; }
    ;

VALUE: RULEREF { $$ = new RuleAtom(@$, std::move($1)); }
     | NUMBER { $$ = $1; }
     | STRCONST { $$ = new StringAtom(@$, std::move($1)); }
     | LISTCONST { $$ = new ListAtom(@$, $1); }
     | NUMBER_RANGE { $$ = $1; }
     | SYMBOL { $$ = new IntAtom(@$, 0); }
     | SELF { $$ = new SelfAtom(@$); }
     | UNDEF { $$ = new UndefAtom(@$); }
     | TRUE { $$ = new BooleanAtom(@$, true); }
     | FALSE { $$ = new BooleanAtom(@$, false); }
     ;

NUMBER: "+" INTCONST %prec UPLUS { $$ = new IntAtom(@$, $2); }
      | "-" INTCONST %prec UMINUS { $$ = new IntAtom(@$, (-1) * $2); }
      | INTCONST { $$ = new IntAtom(@$, $1); }
      | "+" FLOATCONST %prec UPLUS { $$ = new FloatAtom(@$, $2); }
      | "-" FLOATCONST %prec UMINUS { $$ = new FloatAtom(@$, (-1) * $2); }
      | FLOATCONST { $$ = new FloatAtom(@$, $1); }
      | "+" RATIONALCONST %prec UPLUS { $$ = new IntAtom(@$, 0); }
      | "-" RATIONALCONST %prec UMINUS { $$ = new IntAtom(@$, 0); }
      | RATIONALCONST { $$ = new IntAtom(@$, 0); }
      ;

RULEREF: "@" IDENTIFIER { $$ = $2; }
       ;

NUMBER_RANGE: "[" NUMBER DOTDOT NUMBER "]"  {
              if ($2->node_type_ == NodeType::INT_ATOM && $4->node_type_ == NodeType::INT_ATOM) {
                $$ = new NumberRangeAtom(@$, reinterpret_cast<IntAtom*>($2), reinterpret_cast<IntAtom*>($4));
              } else {
                driver.error(@$, "numbers in range expression must be Int");
                $$ = nullptr;
              }
            }
            /*| "[" IDENTIFIER DOTDOT IDENTIFIER "]" */
            ;

LISTCONST: "[" EXPRESSION_LIST "]" { $$ = $2; }
         | "[" "]" { $$ = new std::vector<ExpressionBase*>(); }
         ;


EXPRESSION_LIST: EXPRESSION_LIST_NO_COMMA { $$ = $1; }
               | EXPRESSION_LIST_NO_COMMA "," { $$ = $1; }

EXPRESSION_LIST_NO_COMMA: EXPRESSION_LIST_NO_COMMA"," EXPRESSION 
                        {
                          $$ = $1;
                          $$->push_back($3);
                        }
                        | EXPRESSION
                        {
                          $$ = new std::vector<ExpressionBase*>;
                          $$->push_back($1);
                        }
                        ;


EXPRESSION: EXPRESSION "+" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::ADD); }
          | EXPRESSION "-" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::SUB); }
          | EXPRESSION "*" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::MUL); }
          | EXPRESSION "/" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::DIV); }
          | EXPRESSION "%" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::MOD); }
          | EXPRESSION RATIONAL_DIV EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::RAT_DIV); }
          | EXPRESSION NEQUAL EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::NEQ); }
          | EXPRESSION "=" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::EQ); }
          | EXPRESSION "<" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::LESSER); }
          | EXPRESSION ">" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::GREATER); }
          | EXPRESSION LESSEQ EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::LESSEREQ); }
          | EXPRESSION GREATEREQ EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::GREATEREQ); }
          | EXPRESSION OR EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::OR); }
          | EXPRESSION XOR EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::XOR); }
          | EXPRESSION AND EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::AND); }
          | NOT EXPRESSION
            { $$ = new Expression(@$, $2, nullptr, Expression::Operation::NOT);}
          | ATOM  { $$ = $1; }
          ;

BRACKET_EXPRESSION: "(" EXPRESSION ")"  { $$ = $2; }
                  ;

FUNCTION_SYNTAX: IDENTIFIER { $$ = new FunctionAtom(@$, $1); }
               | IDENTIFIER "(" ")" { $$ = new FunctionAtom(@$, $1); }
               | IDENTIFIER "(" EXPRESSION_LIST ")" { 
                  if (is_builtin_name($1)) {
                    $$ = new BuiltinAtom(@$, $1, $3); 
                  } else {
                    $$ = new FunctionAtom(@$, $1, $3); 
                  }
                }
               ;

RULE_SYNTAX: RULE IDENTIFIER "=" STATEMENT { $$ = new RuleNode(@$, $4, $2); }
           | RULE IDENTIFIER "(" ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $6, $2); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $7, $2, $4); }
/* again, with dump specification */
           | RULE IDENTIFIER DUMPS DUMPSPEC_LIST "=" STATEMENT
              {
                  std::vector<Type*> tmp;
                  $$ = new RuleNode(@$, $6, $2, tmp, $4);
              }
           | RULE IDENTIFIER "(" ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              {
                std::vector<Type*> tmp;
                $$ = new RuleNode(@$, $8, $2, tmp, $6);
              }
           | RULE IDENTIFIER "(" PARAM_LIST ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              {
                std::vector<Type*> tmp;
                $$ = new RuleNode(@$, $9, $2, tmp, $7);
              }
           ;

DUMPSPEC_LIST: DUMPSPEC_LIST "," DUMPSPEC { $$ = std::move($1); $$.push_back($3); }
             | DUMPSPEC { $$ = std::vector<std::pair<std::string,std::vector<std::string>>>(); $$.push_back(std::move($1)); }
             ;

DUMPSPEC: "(" IDENTIFIER_LIST ")" ARROW IDENTIFIER { $$ = std::pair<std::string, std::vector<std::string>>($5, $2); }
        ;

STATEMENT: ASSERT_SYNTAX { $$ = $1; }
         | ASSURE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DIEDIE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | IMPOSSIBLE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DEBUGINFO_SYNTAX { $$ = $1; }
         | PRINT_SYNTAX { $$ = $1; }
         | UPDATE_SYNTAX { $$ = $1; }
         | CASE_SYNTAX { $$ = $1; }
         | CALL_SYNTAX { $$ = $1; }
         | KW_SEQBLOCK_SYNTAX { $$ = $1 ; }
         | SEQBLOCK_SYNTAX { $$ = $1; }
         | KW_PARBLOCK_SYNTAX { $$ = $1; }
         | PARBLOCK_SYNTAX { $$ = $1; }
         | IFTHENELSE { $$ = $1; }
         | LET_SYNTAX { $$ = $1; }
         | PUSH_SYNTAX { $$ = $1; }
         | POP_SYNTAX { $$ = $1; }
         | FORALL_SYNTAX { $$ = $1; }
         | ITERATE_SYNTAX { $$ = $1; }
         | SKIP  { $$ = new AstNode(NodeType::SKIP); }
         | IDENTIFIER  { driver.error(@$, "this call syntax is obsolete, use `call "+$1+"`"); }
         | INTERN EXPRESSION_LIST  { $$ = new AstNode(NodeType::STATEMENT); }
         | OBJDUMP "(" IDENTIFIER ")"   { $$ = new AstNode(NodeType::STATEMENT);}
         ;

ASSERT_SYNTAX: ASSERT EXPRESSION { $$ = new UnaryNode(@$, NodeType::ASSERT, $2); }
             ;
ASSURE_SYNTAX: ASSURE EXPRESSION
             ;

DIEDIE_SYNTAX: DIEDIE EXPRESSION
             | DIEDIE
             ;

/* when symbolic execution:
    * abort trace
    * do not write it
    * no error
  in concrete mode:
    * an error like diedie
*/
IMPOSSIBLE_SYNTAX: IMPOSSIBLE 
         ;

DEBUGINFO_SYNTAX: DEBUGINFO IDENTIFIER DEBUG_ATOM_LIST { $$ = new PrintNode(@$, $2, $3); }
                ;

DEBUG_ATOM_LIST: DEBUG_ATOM_LIST "+" ATOM { $$ = std::move($1); $$.push_back($3); }
               | ATOM { $$.push_back($1); }

PRINT_SYNTAX: PRINT DEBUG_ATOM_LIST { $$ = new PrintNode(@$, $2); }
            ;

UPDATE_SYNTAX: FUNCTION_SYNTAX UPDATE EXPRESSION {
                  if ($1->node_type_ == NodeType::FUNCTION_ATOM) {
                    $$ = new UpdateNode(@$, reinterpret_cast<FunctionAtom*>($1), $3);
                  } else {
                    driver.error(@$, "can only use functions for updates but `"+
                                     std::string("TODO NAME HERE")+"` is a `"+type_to_str($1->node_type_));
                  }
                }
             ;

CASE_SYNTAX: CASE EXPRESSION OF CASE_LABEL_LIST ENDCASE {
                $$ = new CaseNode(@$, $2, $4);
           }
           ;

CASE_LABEL_LIST: CASE_LABEL_LIST CASE_LABEL {
                    $$ = std::move($1);
                    $$.push_back($2);
               }
               | CASE_LABEL {
                    $$ = std::move(std::vector<std::pair<AtomNode*, AstNode*>>());
                    $$.push_back($1);
               }
               ;

CASE_LABEL: CASE_LABEL_DEFAULT { $$ =$1; }
          | CASE_LABEL_NUMBER { $$ = $1; }
          | CASE_LABEL_IDENT  { $$ = $1; }
          | CASE_LABEL_STRING { $$ = $1; }
          ;

CASE_LABEL_DEFAULT: DEFAULT ":" STATEMENT {
                    $$ = std::pair<AtomNode*, AstNode*>(nullptr, $3);
                  }
                  ;

CASE_LABEL_NUMBER: NUMBER ":" STATEMENT {
                    $$ = std::pair<AtomNode*, AstNode*>($1, $3);
                 }
                 ;

CASE_LABEL_IDENT: FUNCTION_SYNTAX ":" STATEMENT {
                    $$ = std::pair<AtomNode*, AstNode*>($1, $3);
                }
                ;

CASE_LABEL_STRING: STRCONST ":" STATEMENT {
                    $$ = std::pair<AtomNode*, AstNode*>(new StringAtom(@$, std::move($1)), $3);
                 }
                 ;

CALL_SYNTAX: CALL "(" EXPRESSION ")" "(" EXPRESSION_LIST ")" { $$ = new CallNode(@$, "", $3, $6); }
           | CALL "(" EXPRESSION ")" { $$ = new CallNode(@$, "", $3); }
           | CALL IDENTIFIER "(" EXPRESSION_LIST ")" { $$ = new CallNode(@$, $2, nullptr, $4); }
           | CALL IDENTIFIER { $$ = new CallNode(@$, $2, nullptr); }
           ;


KW_SEQBLOCK_SYNTAX: SEQBLOCK STATEMENTS ENDSEQBLOCK {
                      $$ = new UnaryNode(@$, NodeType::SEQBLOCK, $2);
                  }
                  ;

SEQBLOCK_SYNTAX: SEQBLOCK_BRACKET STATEMENTS ENDSEQBLOCK_BRACKET {
                      $$ = new UnaryNode(@$, NodeType::SEQBLOCK, $2);
               }
               ;

KW_PARBLOCK_SYNTAX: PARBLOCK STATEMENTS ENDPARBLOCK {
                      $$ = new UnaryNode(@$, NodeType::PARBLOCK, $2);
                  }
                  ;
PARBLOCK_SYNTAX: "{" STATEMENTS "}" {
                    $$ = new UnaryNode(@$, NodeType::PARBLOCK, $2);
               }
               ;

STATEMENTS: STATEMENTS STATEMENT { $1->add($2); $$ = $1; }
          | STATEMENT { $$ = new AstListNode(@$, NodeType::STATEMENTS); $$->add($1); }
          ;

IFTHENELSE: IF EXPRESSION THEN STATEMENT %prec XIF {
                $$ = new IfThenElseNode(@$, $2, $4, nullptr); 
          }
          | IF EXPRESSION THEN STATEMENT ELSE STATEMENT {
                $$ = new IfThenElseNode(@$, $2, $4, $6); 
          }
          ;

LET_SYNTAX: LET IDENTIFIER "=" EXPRESSION IN STATEMENT {
            $$ = new LetNode(@$, Type(TypeType::UNKNOWN), $2, $4, $6);
          }
          | LET IDENTIFIER ":" TYPE_SYNTAX "=" EXPRESSION IN STATEMENT {
            $$ = new LetNode(@$, $4, $2, $6, $8);
          }

          ;

PUSH_SYNTAX: PUSH EXPRESSION INTO FUNCTION_SYNTAX {
                if ($4->node_type_ == NodeType::BUILTIN_ATOM) {
                  driver.error(@$, "cannot pop to builtin `"+$4->name+"`");
                } else {
                    $$ = new PushNode(@$, $2, reinterpret_cast<FunctionAtom*>($4));
                }
          }

           ;

POP_SYNTAX: POP FUNCTION_SYNTAX FROM FUNCTION_SYNTAX {
                if ($2->node_type_ == NodeType::BUILTIN_ATOM) {
                  driver.error(@$, "cannot pop to builtin `"+$2->name+"`");
                } else if ($4->node_type_ == NodeType::BUILTIN_ATOM) {
                  driver.error(@$, "cannot pop to builtin `"+$4->name+"`");
                } else {
                    $$ = new PopNode(@$, reinterpret_cast<FunctionAtom*>($2), reinterpret_cast<FunctionAtom*>($4));
                }
          }
          ;

FORALL_SYNTAX: FORALL IDENTIFIER IN EXPRESSION DO STATEMENT {
                $$ = new ForallNode(@$, $2, $4, $6);
             }
             ;

ITERATE_SYNTAX: ITERATE STATEMENT { $$ = new UnaryNode(@$, NodeType::ITERATE, $2); }
              ;


%%

void yy::casmi_parser::error(const location_type& l,
                              const std::string& m) {
    driver.error (l, m);
}
