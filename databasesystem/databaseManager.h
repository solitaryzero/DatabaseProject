#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "../common.h"
#include "../recordsystem/ColumnInfo.h"
#include "../recordsystem/DataOperands.h"
#include "../recordsystem/RecordConverter.h"
#include "../recordsystem/RID.h"
#include "../recordsystem/TableInfo.h"

class DatabaseManager{
public:
    DatabaseManager();
    ~DatabaseManager();

    void switchDatabase(string dbName);

    shared_ptr<TableInfo> createTable(string tableName);
    shared_ptr<TableInfo> createTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes, vector<int> useIndex);
    shared_ptr<TableInfo> openTable(string tableName);
    void dropTable(string tableName);
    void openDataFile(shared_ptr<TableInfo> p);

    map<string, shared_ptr<TableInfo>> tablePool;
    string databaseName = "";
    string pathBase;
};

#endif