#ifndef COLUMNINFO_H
#define COLUMNINFO_H

#include "../common.h"
#include "TableInfo.h"
#include "DataOperands.h"
#include "../indexsystem/BPlusTree.h"

class TableInfo;

class ColumnInfo{
public:
    ColumnInfo(TableInfo *t, string rawJson);
    ColumnInfo(TableInfo *t, string colName, varTypes colType, int siz, int useIndex = 0);
    ~ColumnInfo();

    json11::Json infoToJson();
    string toJsonDump();
    void setPrimary(bool para);

    TableInfo* tabInfo;
    shared_ptr<BPlusTree> indexTree = nullptr;
    string columnName;
    string columnTypeName;
    varTypes columnType;
    bool isFixed;
    bool allowNull = true;
    bool isPrimary = false;
    int size;
    int useIndex = 0;
};

#endif