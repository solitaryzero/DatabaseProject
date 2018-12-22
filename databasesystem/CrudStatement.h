#ifndef CRUDSTATEMENT_H
#define CRUDSTATEMENT_H

#include "Statement.h"
#include "CrudHelper.h"

class InsertStatement : public Statement{
public:
    string tbName;
    vector<ValueList> vList;

    InsertStatement(string *tableName, vector<ValueList> *valueLists);
    ~InsertStatement();
    void run(DatabaseManager *db);
};

class DeleteStatement : public Statement{
public:
    string tbName;
    vector<WhereClause> wcs;

    DeleteStatement(string *tableName, vector<WhereClause> *whereClauses);
    ~DeleteStatement();
    void run(DatabaseManager *db);
};

class UpdateStatement : public Statement{
public:
    string tbName;
    vector<SetClause> setClauses;
    vector<WhereClause> wcs;

    UpdateStatement(string *tableName, vector<SetClause> *setClauses, vector<WhereClause> *whereClauses);
    ~UpdateStatement();
    void run(DatabaseManager *db);
};

class SelectStatement : public Statement{
public:
    Selector sel;
    vector<string> tList;
    vector<WhereClause> wcs;

    SelectStatement(Selector *sel, vector<string> *tableList, vector<WhereClause> *whereClauses);
    ~SelectStatement();
    void run(DatabaseManager *db);
};

#endif