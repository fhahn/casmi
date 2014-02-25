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
    @$.begin.filename = @$.end.filename = &driver.file;
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

%type <AstNode*> ATOM INIT_SYNTAX BODY_ELEMENT SPECIFICATION
%type <AstListNode*> BODY_ELEMENTS
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
            | BODY_ELEMENT { $$ = new AstListNode(); $$->add($1); }
            ;

BODY_ELEMENT: PROVIDER_SYNTAX { $$ = new AstNode(NodeType::PROVIDER); }
           | OPTION_SYNTAX { $$ = new AstNode(NodeType::OPTION); }
           | ENUM_SYNTAX { $$ = new AstNode(NodeType::ENUM); }
           | FUNCTION_DEFINITION { $$ = new AstNode(NodeType::FUNCTION); }
           | DERIVED_SYNTAX { $$ = new AstNode(NodeType::DERIVED); }
           | INIT_SYNTAX { $$ = $1; }
           | RULE_SYNTAX { $$ = new AstNode(NodeType::RULE); }
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

ATOM: FUNCTION_SYNTAX { $$ = new AstNode(NodeType::ATOM); }
    | VALUE { $$ = new AstNode(NodeType::ATOM); }
    | BRACKET_EXPRESSION { $$ = new AstNode(NodeType::ATOM); }
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

EXPRESSION: EXPRESSION "+" EXPRESSION 
          | EXPRESSION "-" EXPRESSION 
          | EXPRESSION NEQUAL EXPRESSION 
          | EXPRESSION "=" EXPRESSION 
          | EXPRESSION "<" EXPRESSION 
          | EXPRESSION ">" EXPRESSION 
          | EXPRESSION LESSEQ EXPRESSION 
          | EXPRESSION GREATEREQ EXPRESSION 
          | EXPRESSION "*" EXPRESSION 
          | EXPRESSION "/" EXPRESSION 
          | EXPRESSION "%" EXPRESSION 
          | EXPRESSION RATIONAL_DIV EXPRESSION
          | EXPRESSION OR EXPRESSION 
          | EXPRESSION XOR EXPRESSION 
          | EXPRESSION AND EXPRESSION 
          | NOT EXPRESSION 
          | ATOM 
          ;

BRACKET_EXPRESSION: "(" EXPRESSION ")" 
                  ;

FUNCTION_SYNTAX: IDENTIFIER 
               | IDENTIFIER "(" ")" 
               | IDENTIFIER "(" EXPRESSION_LIST ")"
               ;

RULE_SYNTAX: RULE IDENTIFIER "=" STATEMENT 
           | RULE IDENTIFIER "(" ")" "=" STATEMENT 
           | RULE IDENTIFIER "(" PARAM_LIST ")" "=" STATEMENT
/* nochmals, mit dump specification */
           | RULE IDENTIFIER DUMPS DUMPSPEC_LIST "=" STATEMENT 
           | RULE IDENTIFIER "(" ")" DUMPS DUMPSPEC_LIST "=" STATEMENT 
           | RULE IDENTIFIER "(" PARAM_LIST ")" DUMPS DUMPSPEC_LIST "=" STATEMENT
           ;

DUMPSPEC_LIST: DUMPSPEC "," DUMPSPEC_LIST 
             | DUMPSPEC 
             ;

DUMPSPEC: "(" IDENTIFIER_LIST ")" ARROW IDENTIFIER
        ;

STATEMENT: ASSERT_SYNTAX
         | ASSURE_SYNTAX
         | DIEDIE_SYNTAX
         | IMPOSSIBLE_SYNTAX
         | DEBUGINFO_SYNTAX
         | PRINT_SYNTAX
         | UPDATE_SYNTAX
         | CASE_SYNTAX
         | CALL_SYNTAX
         | KW_SEQBLOCK_SYNTAX
         | SEQBLOCK_SYNTAX
         | KW_PARBLOCK_SYNTAX
         | PARBLOCK_SYNTAX
         | IFTHENELSE
         | LET_SYNTAX
         | PUSH_SYNTAX
         | POP_SYNTAX
         | FORALL_SYNTAX
         | ITERATE_SYNTAX
         | SKIP 
         | IDENTIFIER 
         | INTERN EXPRESSION_LIST 
         | OBJDUMP "(" IDENTIFIER ")"  
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

UPDATE_SYNTAX: FUNCTION_SYNTAX UPDATE EXPRESSION
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

KW_PARBLOCK_SYNTAX: PARBLOCK STATEMENTS ENDPARBLOCK 
          ;
PARBLOCK_SYNTAX: "{" STATEMENTS "}" 
               ;

STATEMENTS: STATEMENT STATEMENTS 
          | STATEMENT 
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
