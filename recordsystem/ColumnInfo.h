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
    bool hasForeign = false;
    string foreignTableName = "";
    string foreignColumnName = "";
    vector<pair<string, string>> referedBy;   //by certain table and certain column

    int size;
    int showLength = -1; //int 类型的显示长度
    int useIndex = 0;
};

#endif