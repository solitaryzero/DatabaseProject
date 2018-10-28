#ifndef UNFIXEDRECORDFILE_H
#define UNFIXEDRECORDFILE_H

#include "../common.h"
#include "RID.h"

/*
Unfixed record file format:
    Page0: file header
    Page1-n: data

Data page format:
    --page header--
    data*n
    unused space
    [offset of data[i], size of data[i]](8 bytes)*n
    last free offset(4 bytes)
    used slot numbers in this page(4 bytes)
    --page end--
*/

struct UnfixedRecordFileHeader{
    int isValid;
    int pageNum;
    char tableName[TABLENAMELENGTH];
};

class UnfixedRecordFile{
public:
    UnfixedRecordFile(string filename);
    ~UnfixedRecordFile();

    RID insertData(data_ptr dat);
    bool deleteData(RID target);
    data_ptr getData(RID target);
    RID updateData(RID target, data_ptr dat);

    RID getCurrentRID();
    data_ptr firstData();
    data_ptr nextData();

    void closeFile();

    shared_ptr<BufPageManager> bpm = nullptr;
    UnfixedRecordFileHeader *header = nullptr;
    int fileID, headerIndex;
    int currentPage, currentSlot;
};

#endif