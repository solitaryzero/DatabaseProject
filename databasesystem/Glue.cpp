#include "Glue.h"

DatabaseManager *dbm = nullptr;

void yyerror(const char* s){
    cout << "Error at line: " << yylineno << "\n";
}

void parseSQL(string fileName){
    FILE* in = fopen(fileName.c_str(), "r");
    parseSQL(in);
}

void parseSQL(FILE* in){

    assert(in);
    yyin = in;

    yyrestart(in);
    yyparse();
    fclose(in);
}

void lexSQL(string fileName){
    FILE* in = fopen(fileName.c_str(), "r");
    lexSQL(in);
}

void lexSQL(FILE* in){
    assert(in);

    yyin = in;
    while(yylex() >= 0);
    fclose(in);
}