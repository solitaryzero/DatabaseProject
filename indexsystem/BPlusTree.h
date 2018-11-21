#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "../common.h"
#include "../recordsystem/RID.h"
#include "../recordsystem/UnfixedRecordFile.h"
#include "BPlusTreeFile.h"
#include "BPlusStructures.h"

class BPlusTree{
public:
    BPlusTree(string tableName, string colName);
    ~BPlusTree();

    void insert(data_ptr key, int rid);
    void remove(data_ptr key, int rid);
    bool has(data_ptr key);
    int count(data_ptr key);
    int greaterCount(data_ptr key);

    BPlusNode* currentNode;
    BPlusHeaderPage* header;
    string tableName, colName;

    shared_ptr<BPlusTreeFile> treeFile = nullptr;
    shared_ptr<UnfixedRecordFile> keyFile = nullptr;

    void closeIndex();
    void deleteIndex();
    void insertIntoNonFullPage(data_ptr key, int rid, int pageID); 
};

class BPlusTreeIterator{
public:
    BPlusTreeIterator(BPlusTree* bt);
    BPlusTreeIterator(BPlusTree* bt, BPlusNode* bn, int keyn, int valn);

    data_ptr getKey();
    int getValue();
    void next();
    void nextKey();
    void previous();
    void previousKey();
    bool available();
    void setToBegin();

private:
    BPlusTree* tree;
    BPlusNode* currentNode;
    BPlusOverflowPage* currentOverflowPage;
    int currentKeyPos, currentValuePos, currentCumulation;
};

#endif