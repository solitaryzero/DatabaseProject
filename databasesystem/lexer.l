%option yylineno
%option noyywrap
%option caseless

%x lineComment
%x blockComment

IDENTIFIER              ([A-Za-z][_0-9A-Za-z]*)
VALUE_INT               [0-9]+
VALUE_FLOAT             -?[0-9]+\.[0-9]+
VALUE_STRING            '([^\'\"\r\n]|(\\\')|(\\\"))*'
VALUE_STRING2           "([^\'\"\r\n]|(\\\')|(\\\"))*"
NEWLINE                 (\n|\r\n|\r)
WHITESPACE              [ \t]

%{
#include <cstring>
#include <cstdio>
#include "Glue.h"
#include "myyacc.h"
%}

%%
"--"                    { BEGIN(lineComment); }
<lineComment>{NEWLINE}  { BEGIN(INITIAL); }
<lineComment>.          { /* pass */ }
"/*"                    { BEGIN(blockComment); }
<blockComment>"*/"      { BEGIN(INITIAL); }
<blockComment>\\.       { /* pass escaped character */ }
<blockComment>.         { /* pass */ }

"DATABASE"              { return DATABASE; }
"DATABASES"             { return DATABASES; }
"TABLE"                 { return TABLE; }
"TABLES"                { return TABLES; }
"SHOW"                  { return SHOW; }
"CREATE"                { return CREATE; }
"DROP"                  { return DROP; }
"USE"                   { return USE; }
"PRIMARY"               { return PRIMARY; }
"KEY"                   { return KEY; }
"NOT"                   { return NOT; }
"NULL"                  { return MYNULL; }
"INSERT"                { return INSERT; }
"INTO"                  { return INTO; }
"VALUES"                { return VALUES; }
"DELETE"                { return DELETE; }
"FROM"                  { return FROM; }
"WHERE"                 { return WHERE; }
"UPDATE"                { return UPDATE; }
"SET"                   { return SET; }
"SELECT"                { return SELECT; }
"IS"                    { return IS; }
"INT"                   { return INT; }
"VARCHAR"               { return VARCHAR; }
"DESC"                  { return DESC; }
"INDEX"                 { return INDEX; }
"AND"                   { return AND; }
"OR"                    { return OR; }

"CHAR"                  { return CHAR; }
"FLOAT"                 { return FLOAT; }
"DATE"                  { return DATE; }
"DECIMAL"               { return DECIMAL; }

"FOREIGN"               { return FOREIGN; }

"="                     { return OP_EQ; }
"<>"                    { return OP_NE; }
"!="                    { return OP_NE; }
">"                     { return OP_GT; }
"<"                     { return OP_LT; }
"<="                    { return OP_LE; }
">="                    { return OP_GE; }

"("                     { return yytext[0]; }
")"                     { return yytext[0]; }
";"                     { return yytext[0]; }
","                     { return yytext[0]; }
"."                     { return yytext[0]; }
"*"                     { return yytext[0]; }

"AVG"                   { return AVG; }
"SUM"                   { return SUM; }
"MIN"                   { return MIN; }
"MAX"                   { return MAX; }
"COUNT"                 { return COUNT; }

{NEWLINE}               { /* pass */ }
{WHITESPACE}            { /* pass */ }
{IDENTIFIER}            { yylval.data = new std::string(yytext, yyleng); return IDENTIFIER; }
{VALUE_INT}             { yylval.data = new std::string(yytext, yyleng); return VALUE_INT; }
{VALUE_FLOAT}           { yylval.data = new std::string(yytext, yyleng); return VALUE_FLOAT; }
{VALUE_STRING}          { yylval.data = new std::string(yytext+1, yyleng-2); return VALUE_STRING; }
{VALUE_STRING2}         { yylval.data = new std::string(yytext+1, yyleng-2); return VALUE_STRING; }

.                       { printf("Line %d: unknown token %c", yylineno, yytext[0]); }

%%