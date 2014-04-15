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
}

%token AND OR XOR NOT ASSERT ASSURE DIEDIE IMPOSSIBLE SKIP SEQBLOCK ENDSEQBLOCK
%token PARBLOCK ENDPARBLOCK LET IN IF THEN ELSE PRINT DEBUGINFO DUMPS PUSH INTO
%token POP FROM FORALL ITERATE DO CALL CASE DEFAULT OF ENDCASE INITIALLY FUNCTION
%token DERIVED ENUM RULE PROVIDER INIT OPTION SELF UNDEF TRUE FALSE CASM SYMBOL
%token INTERN RATIONAL_DIV OBJDUMP TYPEANNOTATION ENDTYPEANNOTATION

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

%type <AstNode*> INIT_SYNTAX BODY_ELEMENT SPECIFICATION PARBLOCK_SYNTAX KW_PARBLOCK_SYNTAX
%type <AstNode*> RULE_SYNTAX STATEMENT
%type <UnaryNode*> ASSERT_SYNTAX
%type <AstListNode*> BODY_ELEMENTS STATEMENTS
%type <AtomNode*> NUMBER VALUE
%type <std::pair<ExpressionBase*, ExpressionBase*>> INITIALIZER
%type <std::vector<std::pair<ExpressionBase*, ExpressionBase*>>*> INITIALIZER_LIST INITIALIZERS
%type <ExpressionBase*> EXPRESSION BRACKET_EXPRESSION ATOM
%type <std::vector<ExpressionBase*>*> EXPRESSION_LIST EXPRESSION_LIST_NO_COMMA
%type <UpdateNode*> UPDATE_SYNTAX
%type <INT_T> INTCONST
%type <FLOAT_T> FLOATCONST
%type <std::string> STRCONST
%type <Symbol*> FUNCTION_DEFINITION
%type <FunctionAtom*> FUNCTION_SYNTAX 
%type <std::pair<std::vector<Type>*, Type>> FUNCTION_SIGNATURE
%type <Type> NEW_TYPE_SYNTAX PARAM
%type <std::vector<Type>*> TYPE_IDENTIFIER_STARLIST PARAM_LIST_NO_COMMA PARAM_LIST
%type <std::string> RULEREF
%type <IfThenElseNode*> IFTHENELSE
%type <CallNode*> CALL_SYNTAX

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
           | ENUM_SYNTAX { $$ = new AstNode(NodeType::ENUM); }
           | FUNCTION_DEFINITION {
                $$ = new FunctionDefNode(@$, $1);
                if (!driver.current_symbol_table->add($1)) {
                    driver.error(@$, "redefinition of symbol");
                    // if another symbol with same name exists we need to delete
                    // the symbol here, because it is not inserted in the symbol table
                    delete $1;
                }
            }
           | DERIVED_SYNTAX { $$ = new AstNode(NodeType::DERIVED); }
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

ENUM_SYNTAX: ENUM IDENTIFIER "=" "{" IDENTIFIER_LIST "}";

DERIVED_SYNTAX: DERIVED IDENTIFIER "(" PARAM_LIST ")" "=" EXPRESSION
              | DERIVED IDENTIFIER "=" EXPRESSION
              | DERIVED IDENTIFIER "(" ")" "=" EXPRESSION

              /* again with type syntax */
              | DERIVED IDENTIFIER "(" PARAM_LIST ")" ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              | DERIVED IDENTIFIER ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              | DERIVED IDENTIFIER "(" ")" ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              ;

FUNCTION_DEFINITION: FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
                   { $$ = new Symbol($5, $6.first, $6.second, $7); }
           | FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE
                   { $$ = new Symbol($5, $6.first, $6.second, nullptr); }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
                   { $$ = new Symbol($2, $3.first, $3.second, $4); }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE
                   { $$ = new Symbol($2, $3.first, $3.second, nullptr); }
           ;


IDENTIFIER_LIST: IDENTIFIER "," IDENTIFIER_LIST
               | IDENTIFIER
               | IDENTIFIER ","
               ;

FUNCTION_SIGNATURE: ":" ARROW NEW_TYPE_SYNTAX 
                  /* this constructor is implementation dependant! */
                  { $$ = std::pair<std::vector<Type>*, Type>(nullptr, $3); }
                  | ":" TYPE_IDENTIFIER_STARLIST ARROW NEW_TYPE_SYNTAX
                  { $$ = std::pair<std::vector<Type>*, Type>($2, $4); }
                  ;

PARAM: IDENTIFIER OLD_TYPE_SYNTAX  
     | IDENTIFIER ":" NEW_TYPE_SYNTAX { $$ = $3; }
     | IDENTIFIER
     ;


PARAM_LIST: PARAM_LIST_NO_COMMA { $$ = $1; }
          | PARAM_LIST_NO_COMMA "," { $$ = $1; }

PARAM_LIST_NO_COMMA: PARAM_LIST_NO_COMMA "," PARAM
                   {
                        $$ = $1;
                        $$->push_back($3);
                   }
                   | PARAM
                   { 
                        $$ = new std::vector<Type>;; 
                        $$->push_back($1);
                   }
                   ;

/* TODO: right recursion */
TYPE_IDENTIFIER_STARLIST: NEW_TYPE_SYNTAX "*" TYPE_IDENTIFIER_STARLIST 
                        {
                            $3->insert($3->begin(), $1);
                            $$ = $3;
                        }
                        | NEW_TYPE_SYNTAX "*" 
                        { // TODO: limit memory size
                            $$ = new std::vector<Type>;
                            $$->push_back($1);
                        }
                        | NEW_TYPE_SYNTAX 
                        { // TODO: limit memory size
                            $$ = new std::vector<Type>;
                            $$->push_back($1);
                        }
                        ;

/* new type syntax */
NEW_TYPE_SYNTAX: IDENTIFIER { $$ = str_to_type($1); /* TODO check invalid types */}
               | IDENTIFIER "(" NEW_TYPE_SYNTAX_LIST ")" 
               | IDENTIFIER TYPEANNOTATION IDENTIFIER ENDTYPEANNOTATION
               | IDENTIFIER TYPEANNOTATION "[" TUPLE_LIST "]" ENDTYPEANNOTATION
               | IDENTIFIER "(" NUMBER DOTDOT NUMBER ")"
               ;

NEW_TYPE_SYNTAX_LIST: NEW_TYPE_SYNTAX "," NEW_TYPE_SYNTAX_LIST
                    | NEW_TYPE_SYNTAX ","
                    | NEW_TYPE_SYNTAX
                    ;

/* old type syntax for parameters, rule main(  a /ta: Int/ ) */
OLD_TYPE_SYNTAX: TYPEANNOTATION IDENTIFIER ENDTYPEANNOTATION

TUPLE_LIST: IDENTIFIER "," TUPLE_LIST 
         | IDENTIFIER "," 
         | IDENTIFIER 
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
     | LISTCONST { $$ = new IntAtom(@$, 0); }
     | NUMBER_RANGE { $$ = new IntAtom(@$, 0); }
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
      | FLOATCONST { $$ = new FloatAtom(@$, 0); }
      | "+" RATIONALCONST %prec UPLUS { $$ = new IntAtom(@$, 0); }
      | "-" RATIONALCONST %prec UMINUS { $$ = new IntAtom(@$, 0); }
      | RATIONALCONST { $$ = new IntAtom(@$, 0); }
      ;

RULEREF: "@" IDENTIFIER { $$ = $2; }
       ;

NUMBER_RANGE: "[" NUMBER DOTDOT NUMBER "]" 
            | "[" IDENTIFIER DOTDOT IDENTIFIER "]" 
            ;

LISTCONST: "[" EXPRESSION_LIST "]" 
         | "[" "]" 
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
          | EXPRESSION "*" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::MUL); }
          | EXPRESSION "/" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::DIV); }
          | EXPRESSION "%" EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::MOD); }
          | EXPRESSION RATIONAL_DIV EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::RAT_DIV); }
          | EXPRESSION OR EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::OR); }
          | EXPRESSION XOR EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::XOR); }
          | EXPRESSION AND EXPRESSION
            { $$ = new Expression(@$, $1, $3, Expression::Operation::AND); }
          | NOT EXPRESSION
            { $$ = new Expression(@$, $2, nullptr, Expression::Operation::NOP);}
          | ATOM  { $$ = $1; }
          ;

BRACKET_EXPRESSION: "(" EXPRESSION ")"  { $$ = $2; }
                  ;

FUNCTION_SYNTAX: IDENTIFIER { DEBUG("FUNC "<<$1); $$ = new FunctionAtom(@$, $1); }
               | IDENTIFIER "(" ")" { $$ = new FunctionAtom(@$, $1); }
               | IDENTIFIER "(" EXPRESSION_LIST ")" { $$ = new FunctionAtom(@$, $1, $3); }
               ;

RULE_SYNTAX: RULE IDENTIFIER "=" STATEMENT { $$ = new RuleNode(@$, $4, $2); }
           | RULE IDENTIFIER "(" ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $6, $2); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $7, $2, $4); }
/* again, with dump specification */
           | RULE IDENTIFIER DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new RuleNode(@$, $6, $2); }
           | RULE IDENTIFIER "(" ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new RuleNode(@$, $8, $2); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new RuleNode(@$, $9, $2); }
           ;

DUMPSPEC_LIST: DUMPSPEC_LIST "," DUMPSPEC
             | DUMPSPEC 
             ;

DUMPSPEC: "(" IDENTIFIER_LIST ")" ARROW IDENTIFIER
        ;

STATEMENT: ASSERT_SYNTAX { $$ = $1; }
         | ASSURE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DIEDIE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | IMPOSSIBLE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DEBUGINFO_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | PRINT_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | UPDATE_SYNTAX { $$ = $1; }
         | CASE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | CALL_SYNTAX { $$ = $1; }
         | KW_SEQBLOCK_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | SEQBLOCK_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | KW_PARBLOCK_SYNTAX { $$ = $1; }
         | PARBLOCK_SYNTAX { $$ = $1; }
         | IFTHENELSE { $$ = $1; }
         | LET_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | PUSH_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | POP_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | FORALL_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | ITERATE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | SKIP  { $$ = new AstNode(NodeType::SKIP); }
         | IDENTIFIER  { $$ = new AstNode(NodeType::STATEMENT); }
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

DEBUGINFO_SYNTAX: DEBUGINFO IDENTIFIER DEBUG_ATOM_LIST
                ;

DEBUG_ATOM_LIST: DEBUG_ATOM_LIST "+" ATOM
               | ATOM

PRINT_SYNTAX: PRINT DEBUG_ATOM_LIST
            ;

UPDATE_SYNTAX: FUNCTION_SYNTAX UPDATE EXPRESSION { $$ = new UpdateNode(@$, $1, $3); }
             ;

CASE_SYNTAX: CASE EXPRESSION OF CASE_LABEL_LIST ENDCASE
           ;

CASE_LABEL_LIST: CASE_LABEL CASE_LABEL_LIST 
               | CASE_LABEL 
               ;

CASE_LABEL: CASE_LABEL_DEFAULT
          | CASE_LABEL_NUMBER
          | CASE_LABEL_IDENT
          | CASE_LABEL_STRING
          ;

CASE_LABEL_DEFAULT: DEFAULT ":" STATEMENT
                  ;

CASE_LABEL_NUMBER: NUMBER ":" STATEMENT 
                 ;

CASE_LABEL_IDENT: IDENTIFIER ":" STATEMENT 
                ;

CASE_LABEL_STRING: STRCONST ":" STATEMENT 
                 ;

CALL_SYNTAX: CALL "(" EXPRESSION ")" "(" EXPRESSION_LIST ")" { $$ = new CallNode(@$, "", $3, $6); }
           | CALL "(" EXPRESSION ")" { $$ = new CallNode(@$, "", $3); }
           | CALL IDENTIFIER "(" EXPRESSION_LIST ")" { $$ = new CallNode(@$, $2, nullptr, $4); }
           | CALL IDENTIFIER { $$ = new CallNode(@$, $2, nullptr); }
           ;


KW_SEQBLOCK_SYNTAX: SEQBLOCK STATEMENTS ENDSEQBLOCK 
                  ;

SEQBLOCK_SYNTAX: SEQBLOCK_BRACKET STATEMENTS ENDSEQBLOCK_BRACKET  
               ; 

KW_PARBLOCK_SYNTAX: PARBLOCK STATEMENTS ENDPARBLOCK { $$ = new UnaryNode(@$, NodeType::PARBLOCK, $2); }
          ;
PARBLOCK_SYNTAX: "{" STATEMENTS "}" { $$ = new UnaryNode(@$, NodeType::PARBLOCK, $2); }
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

LET_SYNTAX: LET IDENTIFIER "=" EXPRESSION IN STATEMENT
          | LET IDENTIFIER ":" NEW_TYPE_SYNTAX "=" EXPRESSION IN STATEMENT
          ;

PUSH_SYNTAX: PUSH EXPRESSION INTO IDENTIFIER
           ;

POP_SYNTAX: POP IDENTIFIER FROM IDENTIFIER
          ;

FORALL_SYNTAX: FORALL IDENTIFIER IN EXPRESSION DO STATEMENT
             ;

ITERATE_SYNTAX: ITERATE STATEMENT
          ;


%%

void yy::casmi_parser::error(const location_type& l,
                              const std::string& m) {
    driver.error (l, m);
}
