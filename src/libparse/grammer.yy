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
    #include "libparse/ast.h"
    class casmi_driver;
}

// The parsing context.
%parse-param { casmi_driver& driver }
%lex-param   { casmi_driver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  // Error messages are printed in casmi_driver, I guess location does not 
  // need to know about the filename
  //@$.begin.filename = @$.end.filename = &driver.filename_;
};

%define parse.trace
%define parse.error verbose

%code {
    #include "libparse/driver.h"
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
%type <AstListNode*> BODY_ELEMENTS STATEMENTS
%type <AtomNode*> ATOM
%type <Expression*> EXPRESSION
%type <UpdateNode*> UPDATE_SYNTAX
%type <Value*> NUMBER
%type <Value*> VALUE
%type <uint64_t> INTCONST

%start SPECIFICATION

/* TODO: Check! */
%left UMINUS UPLUS XIF


%%


SPECIFICATION: HEADER BODY_ELEMENTS { driver.result = new AstNode(NodeType::SPECIFICATION); }
             | BODY_ELEMENTS { driver.result = $1; }
             ;

HEADER: CASM IDENTIFIER
      ;

BODY_ELEMENTS: BODY_ELEMENTS BODY_ELEMENT { $1->add($2); $$ = $1; }
            | BODY_ELEMENT { $$ = new AstListNode(NodeType::BODY_ELEMENTS); $$->add($1); }
            ;

BODY_ELEMENT: PROVIDER_SYNTAX { $$ = new AstNode(NodeType::PROVIDER); }
           | OPTION_SYNTAX { $$ = new AstNode(NodeType::OPTION); }
           | ENUM_SYNTAX { $$ = new AstNode(NodeType::ENUM); }
           | FUNCTION_DEFINITION { $$ = new AstNode(NodeType::FUNCTION); }
           | DERIVED_SYNTAX { $$ = new AstNode(NodeType::DERIVED); }
           | INIT_SYNTAX { $$ = $1; }
           | RULE_SYNTAX { $$ = $1; }
           ;

INIT_SYNTAX: INIT IDENTIFIER { $$ = new AstNode(NodeType::INIT); }
           ;

PROVIDER_SYNTAX: PROVIDER IDENTIFIER 
          ;

OPTION_SYNTAX: OPTION IDENTIFIER "." IDENTIFIER IDENTIFIER;

ENUM_SYNTAX: ENUM IDENTIFIER "=" "{" IDENTIFIER_LIST "}";

DERIVED_SYNTAX: DERIVED IDENTIFIER "(" PARAM_LIST ")" "=" EXPRESSION
              | DERIVED IDENTIFIER "=" EXPRESSION
              | DERIVED IDENTIFIER "(" ")" "=" EXPRESSION

              /* jetzt nochmals mit Typangabe */
              | DERIVED IDENTIFIER "(" PARAM_LIST ")" ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              | DERIVED IDENTIFIER ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              | DERIVED IDENTIFIER "(" ")" ":" NEW_TYPE_SYNTAX "=" EXPRESSION
              ;

FUNCTION_DEFINITION: FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
           | FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE
           ;


IDENTIFIER_LIST: IDENTIFIER "," IDENTIFIER_LIST
               | IDENTIFIER
               | IDENTIFIER ","
               ;

FUNCTION_SIGNATURE: ":" ARROW NEW_TYPE_SYNTAX 
                  | ":" TYPE_IDENTIFIER_STARLIST ARROW NEW_TYPE_SYNTAX
                  ;

PARAM: IDENTIFIER OLD_TYPE_SYNTAX 
     | IDENTIFIER ":" NEW_TYPE_SYNTAX 
     | IDENTIFIER 
     ;

PARAM_LIST: PARAM "," PARAM_LIST 
          | PARAM "," 
          | PARAM 
          ;

TYPE_IDENTIFIER_STARLIST: NEW_TYPE_SYNTAX "*" TYPE_IDENTIFIER_STARLIST
                        | NEW_TYPE_SYNTAX "*" 
                        | NEW_TYPE_SYNTAX 
                        ;

/* Die neue Typ-Syntax */
NEW_TYPE_SYNTAX: IDENTIFIER 
               | IDENTIFIER "(" NEW_TYPE_SYNTAX_LIST ")" 
               | IDENTIFIER TYPEANNOTATION IDENTIFIER ENDTYPEANNOTATION
               | IDENTIFIER TYPEANNOTATION "[" TUPLE_LIST "]" ENDTYPEANNOTATION
               | IDENTIFIER "(" NUMBER DOTDOT NUMBER ")"
               ;

NEW_TYPE_SYNTAX_LIST: NEW_TYPE_SYNTAX "," NEW_TYPE_SYNTAX_LIST
                    | NEW_TYPE_SYNTAX ","
                    | NEW_TYPE_SYNTAX
                    ;

/* Alte Typ-Syntax für Parameter, rule main(  a /ta: Int/ ) */
OLD_TYPE_SYNTAX: TYPEANNOTATION IDENTIFIER ENDTYPEANNOTATION

TUPLE_LIST: IDENTIFIER "," TUPLE_LIST 
         | IDENTIFIER "," 
         | IDENTIFIER 
         ;

INITIALIZERS: INITIALLY "{" INITIALIZER_LIST "}"
            | INITIALLY "{" "}"
            ;

INITIALIZER_LIST: INITIALIZER_LIST "," INITIALIZER
                | INITIALIZER_LIST "," 
                | INITIALIZER 
                ;

INITIALIZER: ATOM 
           | ATOM ARROW ATOM 
           ;

ATOM: FUNCTION_SYNTAX { $$ = new AtomNode(nullptr); }
    | VALUE { $$ = new AtomNode($1); }
    | BRACKET_EXPRESSION { $$ = new AtomNode(nullptr); }
    ;

VALUE: RULEREF { $$ = new Value(); }
     | NUMBER { $$ = $1; }
     | STRCONST { $$ = new Value(); }
     | LISTCONST { $$ = new Value(); }
     | NUMBER_RANGE { $$ = new Value(); }
     | SYMBOL { $$ = new Value(); }
     | SELF { $$ = new Value(); }
     | UNDEF { $$ = new Value(); }
     | TRUE { $$ = new Value(); }
     | FALSE { $$ = new Value(); }
     ;

NUMBER: "+" INTCONST %prec UPLUS { $$ = new IntValue($2); }
      | "-" INTCONST %prec UMINUS { $$ = new IntValue($2); }
      | INTCONST { $$ = new IntValue($1); }
      | "+" FLOATCONST %prec UPLUS { $$ = new Value(); }
      | "-" FLOATCONST %prec UMINUS { $$ = new Value(); }
      | FLOATCONST { $$ = new Value(); }
      | "+" RATIONALCONST %prec UPLUS { $$ = new Value(); }
      | "-" RATIONALCONST %prec UMINUS { $$ = new Value(); }
      | RATIONALCONST { $$ = new Value(); }
      ;

RULEREF: "@" IDENTIFIER 
       ;

NUMBER_RANGE: "[" NUMBER DOTDOT NUMBER "]" 
            | "[" IDENTIFIER DOTDOT IDENTIFIER "]" 
            ;

LISTCONST: "[" EXPRESSION_LIST "]" 
         | "[" "]" 
         ;

EXPRESSION_LIST: EXPRESSION "," EXPRESSION_LIST 
               | EXPRESSION "," 
               | EXPRESSION 
               ;

EXPRESSION: EXPRESSION "+" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "-" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION NEQUAL ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "=" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "<" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION ">" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION LESSEQ ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION GREATEREQ ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "*" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "/" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION "%" ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION RATIONAL_DIV ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION OR  ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION XOR ATOM { $$ = new Expression($1, $3);}
          | EXPRESSION AND ATOM { $$ = new Expression($1, $3);}
          | NOT EXPRESSION  { $$ = new Expression($2, nullptr);}
          | ATOM  { $$ = new Expression(nullptr, $1);}
          ;

BRACKET_EXPRESSION: "(" EXPRESSION ")" 
                  ;

FUNCTION_SYNTAX: IDENTIFIER 
               | IDENTIFIER "(" ")" 
               | IDENTIFIER "(" EXPRESSION_LIST ")"
               ;

RULE_SYNTAX: RULE IDENTIFIER "=" STATEMENT { $$ = new UnaryNode(NodeType::RULE, $4); }
           | RULE IDENTIFIER "(" ")" "=" STATEMENT
              { $$ = new UnaryNode(NodeType::RULE, $6); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" "=" STATEMENT
              { $$ = new UnaryNode(NodeType::RULE, $7); }
/* nochmals, mit dump specification */
           | RULE IDENTIFIER DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new UnaryNode(NodeType::RULE, $6); }
           | RULE IDENTIFIER "(" ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new UnaryNode(NodeType::RULE, $8); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
              { $$ = new UnaryNode(NodeType::RULE, $9); }
           ;

DUMPSPEC_LIST: DUMPSPEC "," DUMPSPEC_LIST 
             | DUMPSPEC 
             ;

DUMPSPEC: "(" IDENTIFIER_LIST ")" ARROW IDENTIFIER
        ;

STATEMENT: ASSERT_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | ASSURE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DIEDIE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | IMPOSSIBLE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | DEBUGINFO_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | PRINT_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | UPDATE_SYNTAX { $$ = $1; }
         | CASE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | CALL_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | KW_SEQBLOCK_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | SEQBLOCK_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | KW_PARBLOCK_SYNTAX { $$ = $1; }
         | PARBLOCK_SYNTAX { $$ = $1; }
         | IFTHENELSE { $$ = new AstNode(NodeType::STATEMENT); }
         | LET_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | PUSH_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | POP_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | FORALL_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | ITERATE_SYNTAX { $$ = new AstNode(NodeType::STATEMENT); }
         | SKIP  { $$ = new AstNode(NodeType::STATEMENT); }
         | IDENTIFIER  { $$ = new AstNode(NodeType::STATEMENT); }
         | INTERN EXPRESSION_LIST  { $$ = new AstNode(NodeType::STATEMENT); }
         | OBJDUMP "(" IDENTIFIER ")"   { $$ = new AstNode(NodeType::STATEMENT);}
         ;

ASSERT_SYNTAX: ASSERT EXPRESSION
             ;
ASSURE_SYNTAX: ASSURE EXPRESSION 
             ;

DIEDIE_SYNTAX: DIEDIE 
             | DIEDIE EXPRESSION 
             ;

/* when symbolic execution: abor trace, do not write it, no error, in concrete mode: an error like diedie */
IMPOSSIBLE_SYNTAX: IMPOSSIBLE 
         ;

DEBUGINFO_SYNTAX: DEBUGINFO IDENTIFIER DEBUG_ATOM_LIST
                ;

DEBUG_ATOM_LIST: ATOM "+" DEBUG_ATOM_LIST
               | ATOM

PRINT_SYNTAX: PRINT DEBUG_ATOM_LIST
            ;

UPDATE_SYNTAX: FUNCTION_SYNTAX UPDATE EXPRESSION { $$ = new UpdateNode($3); }
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

CALL_SYNTAX: CALL "(" EXPRESSION ")" "(" EXPRESSION_LIST ")"    
           | CALL "(" EXPRESSION ")"                
           | CALL IDENTIFIER "(" EXPRESSION_LIST ")"
           | CALL IDENTIFIER
           ;


KW_SEQBLOCK_SYNTAX: SEQBLOCK STATEMENTS ENDSEQBLOCK 
                  ;

SEQBLOCK_SYNTAX: SEQBLOCK_BRACKET STATEMENTS ENDSEQBLOCK_BRACKET  
               ; 

KW_PARBLOCK_SYNTAX: PARBLOCK STATEMENTS ENDPARBLOCK { $$ = new UnaryNode(NodeType::PARBLOCK, $2); }
          ;
PARBLOCK_SYNTAX: "{" STATEMENTS "}" { $$ = new UnaryNode(NodeType::PARBLOCK, $2); }
               ;

STATEMENTS: STATEMENTS STATEMENT { $1->add($2); $$ = $1; }
          | STATEMENT { $$ = new AstListNode(NodeType::STATEMENTS); $$->add($1); }
          ;

IFTHENELSE: IF EXPRESSION THEN STATEMENT %prec XIF 
          | IF EXPRESSION THEN STATEMENT ELSE STATEMENT
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
