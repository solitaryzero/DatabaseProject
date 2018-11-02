#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include "../common.h"
#include "ColumnInfo.h"
#include "DataOperands.h"
#include "RecordConverter.h"
#include "RID.h"
#include "TableInfo.h"

class RecordManager{
public:
    RecordManager();

    shared_ptr<TableInfo> newTable(string tableName);
    shared_ptr<TableInfo> newTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes);
    shared_ptr<TableInfo> openTable(string tableName);
    void dropTable(string tableName);
    void openDataFile(shared_ptr<TableInfo> p);

    map<string, shared_ptr<TableInfo>> tablePool;
};

#endif