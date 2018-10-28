#ifndef FIXEDRECORDFILE_H
#define FIXEDRECORDFILE_H

#include "../common.h"
#include "RID.h"

struct FixedRecordFileHeader{
    int isValid;
    int pageNum;
    char tableName[TABLENAMELENGTH];
};

class FixedRecordFile{
public:
    FixedRecordFile(string filename);
    ~FixedRecordFile();

    RID insertData(data_ptr dat);
    bool deleteData(RID target);
    data_ptr getData(RID target);
    RID updateData(RID target, data_ptr dat);

    shared_ptr<BufPageManager> bpm = nullptr;
    FixedRecordFileHeader *header = nullptr;
    int fileID;
};

#endif