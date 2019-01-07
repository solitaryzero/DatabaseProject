#ifndef ABSTRACTRECORDFILE_H
#define ABSTRACTRECORDFILE_H

#include "../common.h"
#include "RID.h"

class AbstractRecordFile{
public:
    virtual ~AbstractRecordFile(){
        
    }

    virtual RID insertData(data_ptr dat) = 0;
    virtual bool deleteData(RID target) = 0;
    virtual data_ptr getData(RID target) = 0;
    virtual RID updateData(RID target, data_ptr dat) = 0;

    virtual RID getCurrentRID() = 0;
    virtual data_ptr firstData() = 0;
    virtual data_ptr nextData() = 0;

    virtual void closeFile() = 0;
};

#endif