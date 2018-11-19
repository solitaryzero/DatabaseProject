#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "../common.h"
#include "../recordsystem/RID.h"
#include "../recordsystem/UnfixedRecordFile.h"
#include "BPlusTreeFile.h"

enum NodeType {UNDEFINED = 0, INTERMEDIATE = 1, LEAF = 2, OVERFLOW = 3};

struct IndexRecord{
    int keyPos;
    int value;  
    //for leaf page: negative -> -overflowpage id; positive -> RID
    //for intermediate page: pointer left to this page
    int count;
};

struct BPlusNode{
    NodeType nodeType;
    int pageId;
    int recCount;
    int prevPage;
    int nextPage;   //-1 for nonexistent
    IndexRecord data[MAXINDEXRECPERPAGE];
};

struct BPlusOverflowPage{
    NodeType nodeType;
    int pageId;
    int recCount;
    int prevPage;
    int nextPage;
    int fatherPage;
    int data[MAXRECPEROVERFLOWPAGE];
};

struct BPlusHeaderPage{
    int rootPageId;
    int firstLeaf;
    int lastLeaf;
    int sum;
};

class BPlusTree{
public:
    BPlusTree(string tableName, string colName);

    void insert(data_ptr key, int rid);
    void remove(data_ptr key, int rid);
    bool has(data_ptr key);
    int count(data_ptr key);
    int greaterCount(data_ptr key);

private:
    int maxPage;
    int rootPageId, firstLeaf, lastLeaf;
    BPlusNode* currentNode;
    BPlusHeaderPage* header;
    string tableName, colName;

    shared_ptr<BPlusTreeFile> treeFile = nullptr;
    shared_ptr<UnfixedRecordFile> keyFile = nullptr;
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
    bool hasNext();
    void setToBegin();

private:
    BPlusTree* tree;
    BPlusNode* currentNode;
    int currentKeyPos, currentValuePos;
};

#endif