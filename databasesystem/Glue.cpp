#include "Glue.h"

DatabaseManager *dbm = nullptr;

void yyerror(const char* s){
    cout << "Error at line: " << yylineno << "\n";
}

void parseSQL(string fileName){
    string fullPath = dbm->pathBase+"../"+fileName;
    FILE* in = fopen(fullPath.c_str(), "r");
    parseSQL(in);
}

void parseSQL(FILE* in){
    assert(in);
    yyin = in;

    yyrestart(in);
    yyparse();
    fclose(in);
    in = NULL;
}

void lexSQL(string fileName){
    string fullPath = dbm->pathBase+"../"+fileName;
    FILE* in = fopen(fullPath.c_str(), "r");
    lexSQL(in);
}

void lexSQL(FILE* in){
    assert(in);

    yyin = in;
    while(yylex() >= 0);
    fclose(in);
}