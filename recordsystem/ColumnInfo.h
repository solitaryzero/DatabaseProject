#ifndef COLUMNINFO_H
#define COLUMNINFO_H

#include "../common.h"
#include "TableInfo.h"
#include "DataOperands.h"

class TableInfo;

class ColumnInfo{
public:
    ColumnInfo(TableInfo *t, string rawJson);
    ColumnInfo(TableInfo *t, string colName, varTypes colType, int siz, int useIndex = 0);
    ~ColumnInfo();

    json11::Json infoToJson();
    string toJsonDump();

    TableInfo* tabInfo;
    string columnName;
    string columnTypeName;
    varTypes columnType;
    bool isFixed;
    int size;
    int useIndex = 0;
};

#endif