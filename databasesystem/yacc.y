%{
#include <string>
#include <vector>
#include <memory>
#include "SystemStatement.h"
#include "CrudStatement.h"
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
%token FOREIGN REFERENCES
%token OP_EQ OP_NE OP_GT OP_LT OP_LE OP_GE
%token AVG SUM MIN MAX COUNT

%union {
    string* data;    //acts as string or as some kind of char[] to store values
    Statement* stmt;
    Field* field;
    vector<Field>* fieldList;
    vector<string>* colNameList;
    Type* type;
    Value* value;
    ValueList* valueList;
    vector<ValueList>* valueLists;
    WhereClause* whereClause;
    vector<WhereClause>* whereClauses;
    vector<SetClause>* setClause;
    vector<string>* tableList;
    Selector* selector;
    Column* col;
    Expr* expr;
    WhereOperands op;
    vector<Column>* colSelector;
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
%type <colNameList> colNames
%type <value> value
%type <valueList> valueList
%type <valueLists> valueLists
%type <whereClause> whereClause
%type <whereClauses> whereClauses
%type <setClause> setClause
%type <tableList> tableList
%type <selector> selector
%type <col> col
%type <expr> expr
%type <op> op
%type <colSelector> colSelector

%%

program         : /* empty */
                    {
                        /* pass */
                    }
                | program stmt 
                    {
                        $2->run(dbm);
                        delete $2;
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
                        $$ = new InsertStatement($3, $5);
                    }
                | DELETE FROM tbName WHERE whereClauses
                    {
                        $$ = new DeleteStatement($3, $5);
                    }
                | UPDATE tbName SET setClause WHERE whereClauses
                    {
                        $$ = new UpdateStatement($2, $4, $6);
                    }
                | SELECT selector FROM tableList WHERE whereClauses
                    {
                        $$ = new SelectStatement($2, $4, $6);
                    }
                | SELECT selector FROM tableList
                    {   
                        vector<WhereClause> *wcs = new vector<WhereClause>();
                        $$ = new SelectStatement($2, $4, wcs);
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
                | PRIMARY KEY '(' colNames ')'
                    {
                        $$ = new Field($4);
                        $$->mode = FIELD_PRIMARY;
                    }
                | FOREIGN KEY '(' colName ')' REFERENCES tbName '(' colName ')'
                    {
                        $$ = new Field($4, $7, $9);
                        $$->mode = FIELD_FOREIGN;
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
                | DATE
                    {
                        $$ = new Type(varTypes::DATE_TYPE);
                    }
                | DECIMAL
                    {
                        $$ = new Type(varTypes::DECIMAL_TYPE);
                    }
                ;
    
valueLists      : '(' valueList ')'
                    {
                        $$ = new vector<ValueList>;
                        $$->push_back(*$2);
                        delete $2;
                    }
                | valueLists ',' '(' valueList ')'
                    {
                        $1->push_back(*$4);
                        $$ = $1;
                        delete $4;
                    }
                ;

valueList       : value
                    {
                        $$ = new ValueList();
                        $$->values.clear();
                        $$->values.push_back(*$1);
                        delete $1;
                    }
                | valueList ',' value
                    {
                        $1->values.push_back(*$3);
                        $$ = $1;
                        delete $3;
                    }
                ;
                
value           : VALUE_INT
                    {
                        $$ = new IntValue($1);
                    }
                | VALUE_STRING
                    {
                        $$ = new StringValue($1);
                    }
                | VALUE_FLOAT
                    {
                        $$ = new FloatValue($1);
                    }
                | MYNULL
                    {
                        $$ = new NullValue();
                    }
                ;

whereClauses    : whereClause
                    {
                        $$ = new vector<WhereClause>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | whereClauses AND whereClause
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        delete $3;
                    }
                | whereClauses OR whereClause
                    {
                        /* pass */
                        $$ = $1;
                    }
                ; 

whereClause     : col op expr
                    {
                        $$ = new WhereClause($1, $2, $3);
                    }
                | col IS NOT MYNULL
                    {
                        $$ = new WhereClause($1, WHERE_OP_NOTNULL);
                    }
                | col IS MYNULL
                    {
                        $$ = new WhereClause($1, WHERE_OP_ISNULL);
                    }
                ;

col             : tbName '.' colName
                    {
                        $$ = new Column($1, $3);
                    }
                | colName
                    {
                        $$ = new Column($1);
                    }
                ;

op              : OP_EQ
                    {
                        $$ = WHERE_OP_EQ;
                    }
                | OP_NE
                    {
                        $$ = WHERE_OP_NE;
                    }
                | OP_GE
                    {
                        $$ = WHERE_OP_GE;
                    }
                | OP_LE
                    {
                        $$ = WHERE_OP_LE;
                    }
                | OP_GT
                    {
                        $$ = WHERE_OP_GT;
                    }
                | OP_LT
                    {
                        $$ = WHERE_OP_LT;
                    }
                ;

expr            : value
                    {
                        $$ = new Expr($1);
                    }
                | col
                    {  
                        $$ = new Expr($1);
                    }
                ;

setClause       : colName OP_EQ value
                    {
                        $$ = new vector<SetClause>();
                        $$->push_back(SetClause($1, $3));
                    }
                | setClause ',' colName OP_EQ value
                    {
                        $$ = $1;
                        $$->push_back(SetClause($3, $5));
                    }
                ;

selector        : '*'
                    {
                        $$ = new WildSelector();
                    }
                | colSelector
                    {
                        $$ = new ColumnSelector($1);
                    }
                ;

colSelector     : col
                    {
                        $$ = new vector<Column>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | colSelector ',' col
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        delete $3;
                    }
                ;

tableList       : tbName
                    {
                        $$ = new vector<string>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | tableList ',' tbName
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        delete $3;
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

colNames        : colName
                    {
                        $$ = new vector<string>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | colNames ',' colName
                    {
                        $$->push_back(*$3);
                        delete $3;
                    }

colName         : IDENTIFIER
                    {
                        $$ = $1;
                    }
                | DATE
                    {
                        printf("[Warning] Keyword date used as column name!\n");
                        $$ = new string("date");
                    }
                ;

%%