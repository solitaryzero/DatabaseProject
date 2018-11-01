#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include "../common.h"
#include "ColumnInfo.h"
#include "DataOperands.h"
#include "FixedRecordFile.h"
#include "UnfixedRecordFile.h"
#include "RecordConverter.h"
#include "RID.h"
#include "TableInfo.h"

class RecordManager{
public:
    void newTable(string tableName);
    void newTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes);
    void openTable(string tableName);
    void dropTable(string tableName);
    void openDataFile();

    bool isFixed = false;
    shared_ptr<TableInfo> tinfo = nullptr;
    shared_ptr<FixedRecordFile> fixedFile = nullptr;
    shared_ptr<UnfixedRecordFile> unfixedFile = nullptr;
    string infoFileName, dataFileName;
};

#endif