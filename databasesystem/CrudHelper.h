#ifndef CRUDHELPER_H
#define CRUDHELPER_H

#include "Statement.h"

class CrudHelper{
public:
    static bool convertible(varTypes to, varTypes from);
    static data_ptr convert(varTypes dest, Value &v, bool &success);

    static const int failed = 1e9;
    static int getCount(shared_ptr<TableInfo> tif, const WhereClause &wc);
    static vector<RID> getRIDsFrom(shared_ptr<TableInfo> tif, const vector<WhereClause> &wcs);
    static vector<RID> getRIDsFrom(shared_ptr<TableInfo> tif, const WhereClause &wc);
    static bool checkCondition(shared_ptr<RecordConverter> cvt, data_ptr data, const WhereClause &wc);
    static void solveForeignKey_delete(DatabaseManager *db, shared_ptr<TableInfo> tif, const vector<RID> &rids);

    static bool getTableName(DatabaseManager *db, Column &col, vector<string> &tables);
    static WhereClause genConstraint(string tableName, string columnName, Value v);
};

#endif