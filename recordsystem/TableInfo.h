#ifndef TABLEINFO_H
#define TABLEINFO_H

#include "../common.h"
#include "ColumnInfo.h"
#include "RecordConverter.h"
#include "AbstractRecordFile.h"
#include "FixedRecordFile.h"
#include "UnfixedRecordFile.h"

class ColumnInfo;
class RecordConverter;

class TableInfo{
public:
    TableInfo(string tableName, bool newTable = false);
    ~TableInfo();

    void addNewColumn(string colName, varTypes colType, int siz, int useIndex = 0);
    string infoToString();
    void writeBack();
    void genConverter();
    int getFixedLength();
    int getFixedRecordLength();
    void showTableInfo();
    void openDataFile();

    string tableName, infoFileName;
    int colNumbers, fixedColNumbers, unfixedColNumbers;
    vector<shared_ptr<ColumnInfo>> colInfos;
    map<string, int> colIndex;
    map<string, shared_ptr<ColumnInfo>> colInfoMapping;
    fstream fs = fstream();
    shared_ptr<RecordConverter> cvt;
    shared_ptr<AbstractRecordFile> dataFile = nullptr;
};

#endif