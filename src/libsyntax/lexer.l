%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "libsyntax/types.h"
#include "libsyntax/driver.h"
#include "libsyntax/lexer_helpers.h"

#include "parser.tab.h"

# undef yywrap
# define yywrap() 1

// The location of the current token.
static yy::location loc;

// driver must be global, because it is needed for YY_INPUT
// defined in src/libsyntax/driver.cpp
extern Driver *global_driver;

#define STRBUF_LENGTH 1000
void yyerror(const char *s);
char strbuf[STRBUF_LENGTH];

#define YY_INPUT(buf,result,max_size) { \
  result = global_driver->get_next_chars(buf, max_size); \
  if (  result <= 0  ) \
      result = YY_NULL; \
}

#define yyterminate() return( yy::casmi_parser::make_END(loc) )

// flex uses `register` keyword, which is deprecated in c++11
#pragma GCC diagnostic ignored "-Wdeprecated-register"

%}

/* use debug option for more output */
%option noyywrap nounput batch noinput

%x LCOMMENT
%x COMMENT
%x STRING

%{
    // Code run each time a pattern is matched.
    # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
    // Code run each time yylex is called.
    loc.step ();
%}

and { return yy::casmi_parser::make_AND(loc); }
or { return yy::casmi_parser::make_OR(loc); }
xor { return yy::casmi_parser::make_XOR(loc); }
not { return yy::casmi_parser::make_NOT(loc); }
assert { return yy::casmi_parser::make_ASSERT(loc); }
assure { return yy::casmi_parser::make_ASSURE(loc); }
diedie { return yy::casmi_parser::make_DIEDIE(loc); }
impossible { return yy::casmi_parser::make_IMPOSSIBLE(loc); }
skip { return yy::casmi_parser::make_SKIP(loc); }
seqblock { return yy::casmi_parser::make_SEQBLOCK(loc); }
endseqblock { return yy::casmi_parser::make_ENDSEQBLOCK(loc); }
par { return yy::casmi_parser::make_PARBLOCK(loc); }
endpar { return yy::casmi_parser::make_ENDPARBLOCK(loc); }
let { return yy::casmi_parser::make_LET(loc); }
in { return yy::casmi_parser::make_IN(loc); }
if { return yy::casmi_parser::make_IF(loc); }
then { return yy::casmi_parser::make_THEN(loc); }
else { return yy::casmi_parser::make_ELSE(loc); }
print { return yy::casmi_parser::make_PRINT(loc); }
debuginfo { return yy::casmi_parser::make_DEBUGINFO(loc); }
dumps { return yy::casmi_parser::make_DUMPS(loc); }
push { return yy::casmi_parser::make_PUSH(loc); }
into { return yy::casmi_parser::make_INTO(loc); }
pop { return yy::casmi_parser::make_POP(loc); }
from { return yy::casmi_parser::make_FROM(loc); }
forall { return yy::casmi_parser::make_FORALL(loc); }
iterate { return yy::casmi_parser::make_ITERATE(loc); }
do { return yy::casmi_parser::make_DO(loc); }
call { return yy::casmi_parser::make_CALL(loc); }
case { return yy::casmi_parser::make_CASE(loc); }
default { return yy::casmi_parser::make_DEFAULT(loc); }
of { return yy::casmi_parser::make_OF(loc); }
endcase { return yy::casmi_parser::make_ENDCASE(loc); }
initially { return yy::casmi_parser::make_INITIALLY(loc); }
function { return yy::casmi_parser::make_FUNCTION(loc); }
derived { return yy::casmi_parser::make_DERIVED(loc); }
enum { return yy::casmi_parser::make_ENUM(loc); }
rule { return yy::casmi_parser::make_RULE(loc); }
provider { return yy::casmi_parser::make_PROVIDER(loc); }
init { return yy::casmi_parser::make_INIT(loc); }
option { return yy::casmi_parser::make_OPTION(loc); }
self { return yy::casmi_parser::make_SELF(loc); }
undef { return yy::casmi_parser::make_UNDEF(loc); }
true { return yy::casmi_parser::make_TRUE(loc); }
false { return yy::casmi_parser::make_FALSE(loc); }
CASM { return yy::casmi_parser::make_CASM(loc); }
symbol { return yy::casmi_parser::make_SYMBOL(loc); }
intern_debug { return yy::casmi_parser::make_INTERN(loc); }
div { return yy::casmi_parser::make_RATIONAL_DIV(loc); }
objdump { return yy::casmi_parser::make_OBJDUMP(loc); }

"+" { return  yy::casmi_parser::make_PLUS(loc); }
"-" { return yy::casmi_parser::make_MINUS(loc); }
"=" { return yy::casmi_parser::make_EQ(loc); }
"(" { return yy::casmi_parser::make_LPAREN(loc); }
")" { return yy::casmi_parser::make_RPAREN(loc); }
"[" { return yy::casmi_parser::make_LSQPAREN(loc); }
"]" { return yy::casmi_parser::make_RSQPAREN(loc); }
"{" { return yy::casmi_parser::make_LCURPAREN(loc); }
"}" { return yy::casmi_parser::make_RCURPAREN(loc); }
"\." { return yy::casmi_parser::make_DOT(loc); }
":" { return yy::casmi_parser::make_COLON(loc); }
"@" { return yy::casmi_parser::make_AT(loc); }
"," { return yy::casmi_parser::make_COMMA(loc); }
"<" { return yy::casmi_parser::make_LESSER(loc); }
">" { return yy::casmi_parser::make_GREATER(loc); }
"*" { return yy::casmi_parser::make_STAR(loc); }
"/" { return yy::casmi_parser::make_SLASH(loc); }
"%" { return yy::casmi_parser::make_PERCENT(loc); }

".." { return yy::casmi_parser::make_DOTDOT(loc); }
"->" { return yy::casmi_parser::make_ARROW(loc); }
":=" { return yy::casmi_parser::make_UPDATE(loc); }
"!=" { return yy::casmi_parser::make_NEQUAL(loc); }
"<=" { return yy::casmi_parser::make_LESSEQ(loc); }
">=" { return yy::casmi_parser::make_GREATEREQ(loc); }
"{|" { return yy::casmi_parser::make_SEQBLOCK_BRACKET(loc); }
"|}" { return yy::casmi_parser::make_ENDSEQBLOCK_BRACKET(loc); }

[a-zA-Z_][a-zA-Z0-9\_]* { return yy::casmi_parser::make_IDENTIFIER(yytext, loc); }

0x[0-9a-fA-F]+ { 
            INT_T val = convert_to_long(yytext+2, 16, driver, loc);
            return yy::casmi_parser::make_INTCONST(val, loc);
            }
[0-9]+ {
          INT_T val = convert_to_long(yytext, 10, driver, loc);
          return yy::casmi_parser::make_INTCONST(val, loc);
       }

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)? { 
            FLOAT_T val = convert_to_float(yytext, driver, loc);
            return yy::casmi_parser::make_FLOATCONST(val, loc);
          }
[0-9]+[/][0-9]+ { 
            rational_t val = convert_to_rational(yytext, driver, loc);
            return yy::casmi_parser::make_RATIONALCONST(val, loc); 
}
[0-9]+r {
          yytext[strlen(yytext)-1] = 0;
          rational_t val = convert_to_rational(yytext, driver, loc);
          return yy::casmi_parser::make_RATIONALCONST(val, loc);
        }

[ \t\r]+ loc.step();/* ignore spaces */

\n+ {loc.lines(yyleng); loc.step(); } /* ignore newlines too, but reset yycolumn */

"//" { BEGIN(LCOMMENT); loc.lines(1); loc.step(); }
<LCOMMENT>.
<LCOMMENT>\n { BEGIN( INITIAL ); }
<LCOMMENT><<EOF>> { BEGIN( INITIAL ); }

"/*" { BEGIN(COMMENT); } /* ignore multi line comment */
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n { }
<COMMENT>([^*]|\n|\r)+|.
<COMMENT><<EOF>> { printf( "Multiline comment not terminated on line %d", yylineno ); exit( 2 ); }

\" { BEGIN(STRING); strbuf[0] = '\0'; }
<STRING>[^\\\"\n]* { strncat( strbuf, yytext, STRBUF_LENGTH-1 ); } /* alle Zeichen */
<STRING>\n { }
<STRING>\\n { strncat( strbuf, "\n", STRBUF_LENGTH-1 ); }
<STRING>\\t { strncat( strbuf, "\t", STRBUF_LENGTH-1 ); }
<STRING>\\\" { strncat( strbuf, "\"", STRBUF_LENGTH-1 ); }
<STRING>\\' { strncat( strbuf, "\'", STRBUF_LENGTH-1 ); }
<STRING><<EOF>> { fprintf( stderr, "unclosed string\n" ); exit( 1 ); }
<STRING>\" { BEGIN(INITIAL); return yy::casmi_parser::make_STRCONST(strbuf, loc); }

. { fprintf( stderr, "Unrecognized character: %s (ASCII-Code: %d) on line %d.\n",
  yytext, *yytext, yylineno ); exit( 1 );
}

%%
