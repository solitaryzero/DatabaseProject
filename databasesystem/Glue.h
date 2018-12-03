#ifndef GLUE_H
#define GLUE_H

#include "DatabaseManager.h"
#include "HelperClasses.h"
#include "Statement.h"
#include "../common.h"

extern FILE* yyin;
extern int yylineno;
extern DatabaseManager *dbm;

void yyerror(const char* s);
int yylex(void);
int yyparse(void);
void yyrestart(FILE* in);

void parseSQL(string fileName);
void parseSQL(FILE* in);
void lexSQL(string fileName);
void lexSQL(FILE* in);

#endif