#ifndef FIXEDRECORDFILE_H
#define FIXEDRECORDFILE_H

#include "../common.h"
#include "RID.h"

/*
Fixed record file format:
    Page0: file header
    Page1-n: data

Data page format:
    --page header--
    FixedRecordPageHeader(8 bytes)
    bitmap(1024 bytes)
    data*n
    --page end--
*/

struct FixedRecordFileHeader{
    int isValid;
    int pageNum;
    int recordLength;
    int availableRecordPerPage;
    char tableName[TABLENAMELENGTH];
};

struct FixedRecordPageHeader{
    int usedSlot;
};

class FixedRecordFile{
public:
    FixedRecordFile(string filename);
    ~FixedRecordFile();

    RID insertData(data_ptr dat);
    bool deleteData(RID target);
    data_ptr getData(RID target);
    RID updateData(RID target, data_ptr dat);

    RID getCurrentRID();
    data_ptr firstData();
    data_ptr nextData();

    void closeFile();
    void setRecordLength(int len);
    void calculateRecordPerPage();

    shared_ptr<BufPageManager> bpm = nullptr;
    FixedRecordFileHeader *header = nullptr;
    int fileID, headerIndex;
    int currentPage, currentSlot;

    int bitmapSize = 1024;
    int dataOffset = sizeof(FixedRecordPageHeader)+bitmapSize;

private:
    int getNextZero(unsigned char *start, int upperBound);
    int getBitAt(unsigned char *start, int place);
    void setBitAt(unsigned char *start, int place, int target);
};

#endif