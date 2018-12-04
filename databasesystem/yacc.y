%{
#include <string>
#include <vector>
#include <memory>
#include "Statement.h"
#include "DatabaseManager.h"
#include "Glue.h"
using namespace std;
%}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE
%token PRIMARY KEY NOT MYNULL
%token INSERT INTO VALUES DELETE FROM WHERE UPDATE SET
%token SELECT IS 
%token DESC INDEX AND OR
%token INT VARCHAR CHAR FLOAT DATE DECIMAL
%token FOREIGN
%token OP_EQ OP_NE OP_GT OP_LT OP_LE OP_GE
%token AVG SUM MIN MAX COUNT

%union {
    string* data;    //acts as string or as some kind of char[] to store values
    Statement* stmt;
    Field* field;
    vector<Field>* fieldList;
    Type* type;
}

%token <data> IDENTIFIER;
%token <data> VALUE_INT;
%token <data> VALUE_FLOAT;
%token <data> VALUE_STRING;

%type <data> dbName tbName colName
%type <stmt> stmt sysStmt dbStmt tbStmt idxStmt
%type <field> field
%type <fieldList> fieldList
%type <type> type

%%

program         : /* empty */
                    {
                        /* pass */
                    }
                | program stmt 
                    {
                        $2->run(dbm);
                    }
                ;

stmt            : sysStmt ';'
                    {
                        $$ = $1;
                    }
                | dbStmt ';'
                    {
                        $$ = $1;
                    }
                | tbStmt ';'
                    {
                        $$ = $1;
                    }
                | idxStmt ';'
                    {
                        $$ = $1;
                    }
                ;

sysStmt         : SHOW DATABASES
                    {
                        $$ = new ShowDatabaseStatement();
                    }
                ;
                
dbStmt          : CREATE DATABASE dbName
                    {
                        $$ = new CreateDatabaseStatement($3);
                    }
                | DROP DATABASE dbName
                    {
                        $$ = new DropDatabaseStatement($3);
                    }
                | USE dbName
                    {
                        $$ = new UseDatabaseStatement($2);
                    }
                | SHOW TABLES
                    {
                        $$ = new ShowTableStatement();
                    }
                ;

tbStmt          : CREATE TABLE tbName '(' fieldList ')'
                    {
                        $$ = new CreateTableStatement($3, $5);
                    }
                | DROP TABLE tbName
                    {
                        $$ = new DropTableStatement($3);
                    }
                | DESC tbName
                    {
                        $$ = new DescTableStatement($2);
                    }
                | INSERT INTO tbName VALUES valueLists
                    {
                        $$ = new EmptyStatement();
                    }
                | DELETE FROM tbName WHERE whereClauses
                    {
                        $$ = new EmptyStatement();
                    }
                | UPDATE tbName SET setClause WHERE whereClauses
                    {
                        $$ = new EmptyStatement();
                    }
                | SELECT selector FROM tableList WHERE whereClauses
                    {
                        $$ = new EmptyStatement();
                    }
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                    {
                        $$ = new CreateIndexStatement($3, $5);
                    }
                | DROP INDEX tbName '(' colName ')'
                    {
                        $$ = new DropIndexStatement($3, $5);
                    }
                ;

fieldList       : field
                    {
                        $$ = new vector<Field>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | fieldList ',' field
                    {
                        $$->push_back(*$3);
                        delete $3;
                    }
                ;

field           : colName type
                    {
                        $$ = new Field($1, $2, true);
                        $$->mode = FIELD_COMMON;
                    }
                | colName type NOT MYNULL
                    {
                        $$ = new Field($1, $2, false);
                        $$->mode = FIELD_NOTNULL;
                    }
                | PRIMARY KEY '(' colName ')'
                    {
                        $$ = new Field($4);
                        $$->mode = FIELD_PRIMARY;
                    }
                ;

type            : INT '(' VALUE_INT ')'
                    {
                        $$ = new Type(varTypes::INT_TYPE, atoi($3->c_str()));
                        delete($3);
                    }
                | CHAR '(' VALUE_INT ')'
                    {
                        $$ = new Type(varTypes::CHAR_TYPE, atoi($3->c_str()));
                        delete($3);
                    }
                | VARCHAR '(' VALUE_INT ')'
                    {
                        $$ = new Type(varTypes::VARCHAR_TYPE, atoi($3->c_str()));
                        delete($3);
                    }
                | FLOAT
                    {
                        $$ = new Type(varTypes::FLOAT_TYPE);
                    }
                ;
    
valueLists      : '(' valueList ')'
                    {

                    }
                | valueLists ',' '(' valueList ')'
                    {

                    }
                ;

valueList       : value
                    {

                    }
                | valueList ',' value
                    {

                    }
                ;
                
value           : VALUE_INT
                    {
                        
                    }
                | VALUE_STRING
                    {
                        
                    }
                | VALUE_FLOAT
                    {
                        
                    }
                | MYNULL
                    {

                    }
                ;

whereClauses    : whereClause
                    {

                    }
                | whereClause AND whereClause
                    {

                    }
                | whereClause OR whereClause
                    {
                        
                    }
                ; 

whereClause     : col op expr
                    {

                    }
                | col IS NOT MYNULL
                    {

                    }
                | col IS MYNULL
                    {

                    }
                ;

col             : tbName '.' colName
                    {

                    }
                | colName
                    {

                    }
                ;

op              : OP_EQ
                    {

                    }
                | OP_NE
                    {

                    }
                | OP_GE
                    {

                    }
                | OP_LE
                    {

                    }
                | OP_GT
                    {

                    }
                | OP_LT
                    {

                    }
                ;

expr            : value
                    {

                    }
                | col
                    {

                    }
                ;

setClause       : colName OP_EQ value
                    {

                    }
                | setClause ',' colName OP_EQ value
                    {

                    }
                ;

selector        : '*'
                    {

                    }
                | colSelector
                    {

                    }
                ;

colSelector     : col
                    {

                    }
                | colSelector ',' col
                    {

                    }
                ;

tableList       : tbName
                    {

                    }
                | tableList ',' tbName
                    {

                    }

dbName          : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

tbName          : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

colName         : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

%%