#ifndef TABLEINFO_H
#define TABLEINFO_H

#include "../common.h"
#include "ColumnInfo.h"

class ColumnInfo;

class TableInfo{
public:
    TableInfo(string tableName, bool newTable = false);
    ~TableInfo();

    void addNewColumn(string colName, varTypes colType, int siz);
    string infoToString();
    void writeBack();

    string tableName, infoFileName;
    int colNumbers, fixedColNumbers, unfixedColNumbers;
    vector<shared_ptr<ColumnInfo>> colInfos;
    map<string, int> colIndex;
    map<string, shared_ptr<ColumnInfo>> colInfoMapping;
    fstream fs = fstream();
};

#endif