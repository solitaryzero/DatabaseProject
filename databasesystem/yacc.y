%{
#include string

%}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE
%token PRIMARY KEY NOT NULL
%token INSERT INTO VALUES DELETE FROM WHERE UPDATE SET
%token SELECT IS 
%token DESC INDEX AND OR
%token INT VARCHAR CHAR FLOAT DATE DECIMAL
%token FOREIGN
%token OP_EQ OP_NE OP_GT OP_LT OP_LE OP_GE
%token AVG SUM MIN MAX COUNT

%union {
    std::string* data;    //acts as string or as some kind of char[] to store values
}

%token <data> IDENTIFIER;
%token <data> VALUE_INT;
%token <data> VALUE_FLOAT;
%token <data> VALUE_STRING;

%%

program         : /* empty */
                    {

                    }
                | program stmt 
                    {

                    
                    }
                ;

stmt            : sysStmt ';'
                    {

                    }
                | dbStmt ';'
                    {

                    }
                | tbStmt ';'
                    {

                    }
                | idxStmt ';'
                    {

                    }
                ;

sysStmt         : SHOW DATABASES
                    {

                    }
                ;
                
dbStmt          : CREATE DATABASE dbName
                    {

                    }
                | DROP DATABASE dbName
                    {

                    }
                | USE dbName
                    {

                    }
                | SHOW TABLES
                    {

                    }
                ;

tbStmt          : CREATE TABLE tbName '(' fieldList ')'
                    {

                    }
                | DROP TABLE tbName
                    {

                    }
                | DESC tbName
                    {

                    }
                | INSERT INTO tbName VALUES valueLists
                    {

                    }
                | DELETE FROM tbName WHERE whereClauses
                    {

                    }
                | UPDATE tbName SET setClause WHERE whereClauses
                    {

                    }
                | SELECT selector FROM tableList WHERE whereClauses
                    {

                    }
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                    {

                    }
                | DROP INDEX tbName '(' colName ')'
                    {

                    }
                ;

fieldList       : field
                    {
                        
                    }
                | fieldList ',' field
                    {

                    }
                ;

field           : colName type
                    {

                    }
                | colName type NOT NULL
                    {

                    }
                | PRIMARY KEY '(' colName ')'
                    {

                    }
                ;

type            : INT '(' VALUE_INT ')'
                    {

                    }
                | VARCHAR '(' VALUE_INT ')'
                    {

                    }
                | FLOAT
                    {

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
                | NULL
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
                | col IS NOT NULL
                    {

                    }
                | col IS NULL
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

                    }
                ;

tbName          : IDENTIFIER
                    {

                    }
                ;

colName         : IDENTIFIER
                    {

                    }
                ;

%%