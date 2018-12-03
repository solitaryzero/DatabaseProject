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

    void showDatabases();
    void switchDatabase(string dbName);
    void createDatabase(string dbName);
    void dropDatabase(string dbName);

    void showTables();
    shared_ptr<TableInfo> createTable(string tableName);
    shared_ptr<TableInfo> createTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes);
    shared_ptr<TableInfo> openTable(string tableName);
    void dropTable(string tableName);
    void descTable(string tableName);
    void openDataFile(shared_ptr<TableInfo> p);

    bool addIndex(string tableName, string colName, int mode=1);
    bool dropIndex(string tableName, string colName);

    map<string, shared_ptr<TableInfo>> tablePool;
    string databaseName = "";
    string pathBase;
};

#endif