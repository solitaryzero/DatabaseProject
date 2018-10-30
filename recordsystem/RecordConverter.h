#ifndef RECORDCONVERTER_H
#define RECORDCONVERTER_H

#include "../common.h"
#include "TableInfo.h"

class RecordConverter{
public:
    RecordConverter();
    ~RecordConverter();
    data_ptr toByteArray();
    void fromByteArray(data_ptr dat);

    shared_ptr<TableInfo> tinfo = nullptr;
    vector<data_ptr> values;
};

#endif