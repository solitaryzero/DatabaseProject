#ifndef RECORDCONVERTER_H
#define RECORDCONVERTER_H

#include "../common.h"
#include "TableInfo.h"

class TableInfo;

class RecordConverter{
public:
    RecordConverter(TableInfo *t);
    ~RecordConverter();
    data_ptr toByteArray();
    void fromByteArray(data_ptr dat);
    void fromNameValueMap(map<string, data_ptr> vmap);
    void fromIndexValueMap(map<int, data_ptr> vmap);

    data_ptr getRawData(string colName);
    data_ptr getRawData(int colIndex);
    void setRawData(string colName, data_ptr dat);
    void setRawData(int colIndex, data_ptr dat);

    int getInt(string colName);
    int getInt(int colIndex);
    void setInt(string colName, int dat);
    void setInt(int colIndex, int dat);

    float getFloat(string colName);
    float getFloat(int colIndex);
    void setFloat(string colName, float dat);
    void setFloat(int colIndex, float dat);

    string getChar(string colName, bool strip = false);
    string getChar(int colIndex, bool strip = false);
    void setChar(string colName, string dat, bool padding = true);
    void setChar(int colIndex, string dat, bool padding = true);

    void setNull(string colName);
    void setNull(int colIndex);
    bool isNull(string colName);
    bool isNull(int colIndex);

    TableInfo *tinfo;
    vector<data_ptr> values;
    int fixedLength;
    //对于定长列，值为距离定长数据块头位置;对于非定长列，值为它"是第几个非定长列"的相反数．
    vector<int> colOffset;
};

#endif