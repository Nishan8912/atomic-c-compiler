%{
#include "y.tab.h"
#include <stdlib.h>
#include <string.h>
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

%%

"program"           { return KWPROGRAM; }
"function"          { return KWFUNCTION; }
"call"              { return KWCALL; }
"global"            { return KWGLOBAL; }
"local"             { return KWLOCAL; }
"int"               { return KWINT; }
"string"            { return KWSTRING; }
"if"                { return KWIF; }
"then"              { return KWTHEN; }
"else"              { return KWELSE; }
"while"             { return KWWHILE; }
"do"                { return KWDO; }
"return"            { return KWRETURN; }
"returnvalue"       { return KWRETURNVAL; }

"<="                { yylval.ival = 256; return RELOP; }
">="                { yylval.ival = 257; return RELOP; }
"=="                { yylval.ival = '='; return RELOP; }
"!="                { yylval.ival = '!'; return RELOP; }
"<"                 { yylval.ival = '<'; return RELOP; }
">"                 { yylval.ival = '>'; return RELOP; }

"="                 { return EQUALS; }
"+"|"-"|"*"|"/"|"%" { yylval.ival = yytext[0]; return ADDOP; }
"&"|"|"|"^"         { yylval.ival = yytext[0]; return ADDOP; }
"!"|"~"             { yylval.ival = yytext[0]; return ADDOP; }

","                 { return COMMA; }
";"                 { return SEMICOLON; }
"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"["                 { return LBRACKET; }
"]"                 { return RBRACKET; }

[0-9]+              { yylval.ival = atoi(yytext); return NUMBER; }
\"[^\"]*\"          { yylval.str = strdup(yytext); return STRING; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return ID; }

[ \t\r\n]+          { /* skip whitespace */ }
.                   { printf("Unexpected character: %s\n", yytext); exit(1); }

%%
