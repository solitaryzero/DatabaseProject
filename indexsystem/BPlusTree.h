#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "../common.h"
#include "../recordsystem/RID.h"
#include "../recordsystem/UnfixedRecordFile.h"

class BPlusTree{
public:
    BPlusTree(string tableName, string colName);

    void insert(data_ptr key, int rid);
    void remove(data_ptr key, int rid);
    bool has(data_ptr key);
    int count(data_ptr key);
    int greaterCount(data_ptr key);
};

#endif