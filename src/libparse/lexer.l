%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "libparse/lexer_helper.h"


#define STRBUF_LENGTH 1000
void yyerror(const char *s);
extern int yycolumn;
char strbuf[STRBUF_LENGTH];
extern int linenumber;

extern "C" int yylex(void);

#define YY_NO_INPUT 1

#define YY_INPUT(buf,result,max_size)  {\
    result = get_next_char(buf, max_size); \
    if (  result <= 0  ) \
        result = YY_NULL; \
    }
%}

%option nounput

%x LCOMMENT
%x COMMENT
%x STRING

%%

and { begin_token("and"); return AND; }
or { begin_token("or"); return OR; }
xor { return XOR; }
not { return NOT; }
assert { return ASSERT; }
assure { return ASSURE; }
diedie { return DIEDIE; }
impossible { return IMPOSSIBLE; }
skip { return SKIP; }
seqblock { return SEQBLOCK; }
endseqblock { return ENDSEQBLOCK; }
par { return PARBLOCK; }
endpar { return ENDPARBLOCK; }
let { return LET; }
in { return IN; }
if { return IF; }
then { return THEN; }
else { return ELSE; }
print { return PRINT; }
debuginfo { return DEBUGINFO; }
dumps { return DUMPS; }
push { return PUSH; }
into { return INTO; }
pop { return POP; }
from { return FROM; }
forall { return FORALL; }
iterate { return ITERATE; }
do { return DO; }
call { return CALL; }
case { return CASE; }
default { return DEFAULT; }
of { return OF; }
endcase { return ENDCASE; }
initially { return INITIALLY; }
function { return FUNCTION; }
derived { return DERIVED; }
enum { return ENUM; }
rule { return RULE; }
provider { return PROVIDER; }
init { return INIT; }
option { return OPTION; }
self { return SELF; }
undef { return UNDEF; }
true { return TRUE; }
false { return FALSE; }
CASM { begin_token("CASM"); return CASM; }
symbol { return SYMBOL; }
intern_debug { return INTERN; }
div { return RATIONAL_DIV; }
objdump { return OBJDUMP; }


"/*ta:" { return TYPEANNOTATION; }
"*/" { return ENDTYPEANNOTATION; }

"+" { begin_token("+"); return '+'; }
"-" { return '-'; }
"=" { return '='; }
"(" { return '('; }
")" { return ')'; }
"[" { return '['; }
"]" { return ']'; }
"{" { return '{'; }
"}" { return '}'; }
"\." { return '.'; }
":" { return ':'; }
"@" { return '@'; }
"," { return ','; }
"<" { return '<'; }
">" { return '>'; }
"*" { return '*'; }
"/" { return '/'; }
"%" { return '%'; }

".." { return DOTDOT; }
"->" { return ARROW; }
":=" { return UPDATE; }
"!=" { return NEQUAL; }
"<=" { return LESSEQ; }
">=" { return GREATEREQ; }
"{|" { return SEQBLOCK_BRACKET; }
"|}" { return ENDSEQBLOCK_BRACKET; }

[a-zA-Z_][a-zA-Z0-9\_]* { begin_token(yytext); yylval.name = strdup( yytext ); return IDENTIFIER; }

0x[0-9a-fA-F]+ { errno = 0; yylval.value = strtoull (yytext+2, NULL, 16); if (errno != 0) { /*err(1, "strtoull error in line %d", yylineno);*/ } ; return INTCONST; }
[0-9]+ { errno = 0; yylval.value = strtoull( yytext, NULL, 10 ); if (errno != 0) { /*err(1, "strtoull error in line %d", yylineno); */ }; return INTCONST; }

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)? { errno = 0; yylval.float_value = strtod(yytext, NULL); if (errno != 0) { /*err(1, "strtod error in line %d", yylineno); */}; return FLOATCONST; }
[0-9]+[/][0-9]+ { /*init_rational(yylval.rational_value, yytext);*/ return RATIONALCONST; }
[0-9]+r { /*init_rational(yylval.rational_value, yytext); */return RATIONALCONST; }



[ \t\r]+ /* ignore spaces */

\n { yycolumn = 1; linenumber += 1 ;} /* ignore newlines too, but reset yycolumn */

"//" { BEGIN(LCOMMENT); }
<LCOMMENT>.
<LCOMMENT>\n { yycolumn = 1; BEGIN( INITIAL ); }
<LCOMMENT><<EOF>> { BEGIN( INITIAL ); }

"/*" { BEGIN(COMMENT); } /* ignore multi line comment */
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n { yycolumn = 1; }
<COMMENT>([^*]|\n|\r)+|.
<COMMENT><<EOF>> { printf( "Multiline comment not terminated on line %d", yylineno ); exit( 2 ); }

\" { BEGIN(STRING); strbuf[0] = '\0'; }
<STRING>[^\\\"\n]* { strncat( strbuf, yytext, STRBUF_LENGTH-1 ); } /* alle Zeichen */
<STRING>\n { yycolumn = 1; }
<STRING>\\n { strncat( strbuf, "\n", STRBUF_LENGTH-1 ); }
<STRING>\\t { strncat( strbuf, "\t", STRBUF_LENGTH-1 ); }
<STRING>\\\" { strncat( strbuf, "\"", STRBUF_LENGTH-1 ); }
<STRING>\\' { strncat( strbuf, "\'", STRBUF_LENGTH-1 ); }
<STRING><<EOF>> { fprintf( stderr, "unclosed string\n" ); exit( 1 ); }
<STRING>\" { yylval.name = strdup( strbuf );  BEGIN(INITIAL); return STRCONST; }

. { fprintf( stderr, "Unrecognized character: %s (ASCII-Code: %d) on line %d.\n",
  yytext, *yytext, yylineno ); exit( 1 );
}

