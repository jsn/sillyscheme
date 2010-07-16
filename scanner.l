%{
#include <stdio.h>
#include "scheme.h"

#define YY_EXTRA_TYPE   char **
#define RETVAL(typ) { *yyextra = yytext ; return typ ; }
%}

%option reentrant header-file="scanner.h" prefix="scml_" noyywrap
%x string

WORD   [^ \t\n\r\"\'`,\(\)\#;]

%%

"0" |
[-+]?[1-9][0-9]*                                RETVAL(FIXNUM)
[-+]?([0-9]+"."[0-9]*) |
[-+]?([0-9]+("."[0-9]*)?[eE][+-]?[1-9][0-9]*)   RETVAL(FLOAT)
";"[^\n]*\n                                     /* Eat up one-line comments. */
[ \t\n]+	                                /* Eat up white space. */
\'|\(|\)|\.|\`|",@"|\,                          RETVAL(SPECIAL)
{WORD}+                                         RETVAL(SYMBOL)
\#[tT]                                          RETVAL(BOOL)
\#[fF]                                          {*yyextra = NULL;return BOOL;}
"#\\".                                          RETVAL(CHAR)
\"                                              BEGIN(string) ;
<string>([^\\\"]|\\.)*                          RETVAL(STRING)
<string>\"                                      BEGIN(INITIAL) ;
.	                                        RETVAL(1000)

%%
