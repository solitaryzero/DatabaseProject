#ifndef UNFIXEDRECORDFILE_H
#define UNFIXEDRECORDFILE_H

#include "../common.h"
#include "RID.h"

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