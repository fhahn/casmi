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

    #include "libparse/ast.h"
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
%type <std::vector<Expression*>*> EXPRESSION_LIST EXPRESSION_LIST_NO_COMMA
%type <UpdateNode*> UPDATE_SYNTAX
%type <AtomNode*> NUMBER
%type <AtomNode*> VALUE
%type <uint64_t> INTCONST
%type <Symbol*> FUNCTION_DEFINITION
%type <SymbolUsage*> FUNCTION_SYNTAX 
%type <std::pair<std::vector<Type>*, Type>> FUNCTION_SIGNATURE
%type <Type> NEW_TYPE_SYNTAX
%type <std::vector<Type>*> TYPE_IDENTIFIER_STARLIST

%start SPECIFICATION

/* TODO: Check! */
%right UMINUS 
%right UPLUS
%left XIF

%precedence THEN ELSE

%precedence UPDATE PRINT ASSURE ASSERT DIEDIE NOT

%precedence ","
%precedence FLOATCONST INTCONST STRCONST RATIONALCONST IDENTIFIER
%precedence AND OR XOR
%precedence RATIONAL_DIV  NEQUAL LESSEQ GREATEREQ "=" "<" ">" "*"  "/" "%"
%precedence "-" "+"

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
                $$ = new AstNode(NodeType::FUNCTION);
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
                   { $$ = new Symbol($5, $6.first, $6.second); }
           | FUNCTION "(" IDENTIFIER_LIST ")" IDENTIFIER FUNCTION_SIGNATURE
                   { $$ = new Symbol($5, $6.first, $6.second); }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE INITIALIZERS
                   { $$ = new Symbol($2, $3.first, $3.second); }
           | FUNCTION IDENTIFIER FUNCTION_SIGNATURE
                   { $$ = new Symbol($2, $3.first, $3.second); }
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
     | IDENTIFIER ":" NEW_TYPE_SYNTAX 
     | IDENTIFIER 
     ;

PARAM_LIST: PARAM "," PARAM_LIST 
          | PARAM "," 
          | PARAM 
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

ATOM: FUNCTION_SYNTAX { $$ = create_atom(@$, $1); }
    | VALUE { $$ = $1; }
    | BRACKET_EXPRESSION { $$ = new AtomNode(); }
    ;

VALUE: RULEREF { $$ = create_atom(@$, 0); }
     | NUMBER { $$ = $1; }
     | STRCONST { $$ = create_atom(@$, 0); }
     | LISTCONST { $$ = create_atom(@$, 0); }
     | NUMBER_RANGE { $$ = create_atom(@$, 0); }
     | SYMBOL { $$ = create_atom(@$, 0); }
     | SELF { $$ = create_atom(@$, 0); }
     | UNDEF { $$ = create_atom(@$); }
     | TRUE { $$ = create_atom(@$, 0); }
     | FALSE { $$ = create_atom(@$, 0); }
     ;

NUMBER: "+" INTCONST %prec UPLUS { $$ = create_atom(@$, $2); }
      | "-" INTCONST %prec UMINUS { $$ = create_atom(@$, $2); }
      | INTCONST { $$ = create_atom(@$, $1); }
      | "+" FLOATCONST %prec UPLUS { $$ = create_atom(@$, 0); }
      | "-" FLOATCONST %prec UMINUS { $$ = create_atom(@$, 0); }
      | FLOATCONST { $$ = create_atom(@$, 0); }
      | "+" RATIONALCONST %prec UPLUS { $$ = create_atom(@$, 0); }
      | "-" RATIONALCONST %prec UMINUS { $$ = create_atom(@$, 0); }
      | RATIONALCONST { $$ = create_atom(@$, 0); }
      ;

RULEREF: "@" IDENTIFIER 
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
                          $$ = new std::vector<Expression*>;
                          $$->push_back($1);
                        }
                        ;


EXPRESSION: EXPRESSION "+" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "-" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION NEQUAL ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "=" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "<" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION ">" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION LESSEQ ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION GREATEREQ ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "*" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "/" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION "%" ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION RATIONAL_DIV ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION OR  ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION XOR ATOM { $$ = new Expression(@$, $1, $3);}
          | EXPRESSION AND ATOM { $$ = new Expression(@$, $1, $3);}
          | NOT EXPRESSION  { $$ = new Expression(@$, $2, nullptr);}
          | ATOM  { $$ = new Expression(@$, nullptr, $1);}
          ;

BRACKET_EXPRESSION: "(" EXPRESSION ")" 
                  ;

FUNCTION_SYNTAX: IDENTIFIER { $$ = new SymbolUsage(@$, $1); }
               | IDENTIFIER "(" ")" { $$ = new SymbolUsage(@$, $1); }
               | IDENTIFIER "(" EXPRESSION_LIST ")" { $$ = new SymbolUsage(@$, $1, $3); }
               ;

RULE_SYNTAX: RULE IDENTIFIER "=" STATEMENT { $$ = new RuleNode(@$, $4, $2); }
           | RULE IDENTIFIER "(" ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $6, $2); }
           | RULE IDENTIFIER "(" PARAM_LIST ")" "=" STATEMENT
              { $$ = new RuleNode(@$, $7, $2); }
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

CALL_SYNTAX: CALL "(" EXPRESSION ")" "(" EXPRESSION_LIST ")"
           | CALL "(" EXPRESSION ")"
           | CALL IDENTIFIER "(" EXPRESSION_LIST ")"
           | CALL IDENTIFIER
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
