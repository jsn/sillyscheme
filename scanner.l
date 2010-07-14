%{
/* Need this for the call to atof() below. */
#include <stdio.h>
#include "scheme.h"

#define YY_EXTRA_TYPE   char **
#define RETVAL(typ) { *yyextra = yytext ; return typ ; }

%}

%option reentrant header-file="scanner.h" prefix="scml_" noyywrap
%x string

WORD   [^ \t\n\r\"\'\(\)\#;]

%%


[-+]?[1-9][0-9]*                                RETVAL(FIXNUM)
[-+]?([0-9]+"."[0-9]*) |
[-+]([0-9]+("."[0-9]*)?[eE][+-]?[1-9][0-9]*)    RETVAL(FLOAT)
";"[^\n]*\n                                     /* Eat up one-line comments. */
[ \t\n]+	                                /* Eat up white space. */
{WORD}+                                         RETVAL(SYMBOL)
"#\\".                                          RETVAL(CHAR)
\"   { BEGIN string ; }
\'|\(|\)                                        RETVAL(SPECIAL)
<string>([^\\\"]|\\.)*                          RETVAL(STRING)
<string>\" BEGIN(INITIAL) ;
.	                                        RETVAL(1000)

%%

#ifdef  MAIN

int main(int argc, char *argv[])
{
    ++argv, --argc;  /* Skip over program name. */
    if (argc > 0)
        yyin = fopen(argv[0], "r");
    else
        yyin = stdin;


    yylex();
}

#endif
