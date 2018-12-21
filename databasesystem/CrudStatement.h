#ifndef CRUDSTATEMENT_H
#define CRUDSTATEMENT_H

#include "Statement.h"

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
    UpdateStatement(string *tableName, SetClause *setClause, vector<WhereClause> *whereClauses);
    ~UpdateStatement();
    void run(DatabaseManager *db);
};

class SelectStatement : public Statement{
public:
    SelectStatement(Selector *sel, vector<string> *tableList, vector<WhereClause> *whereClauses);
    ~SelectStatement();
    void run(DatabaseManager *db);
};

class CrudHelper{
public:
    static const int failed = 1e9;
    static int getCount(shared_ptr<TableInfo> tif, const WhereClause &wc);
    static vector<RID> getRIDsFrom(shared_ptr<TableInfo> tif, const vector<WhereClause> &wcs);
    static vector<RID> getRIDsFrom(shared_ptr<TableInfo> tif, const WhereClause &wc);
    static bool checkCondition(shared_ptr<RecordConverter> cvt, data_ptr data, const WhereClause &wc);
    static void solveForeignKey_delete(DatabaseManager *db, shared_ptr<TableInfo> tif, const vector<RID> &rids);
};

#endif