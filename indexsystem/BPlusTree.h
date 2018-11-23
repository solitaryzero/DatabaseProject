#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "../common.h"
#include "../recordsystem/RID.h"
#include "../recordsystem/UnfixedRecordFile.h"
#include "../recordsystem/DataOperands.h"
#include "BPlusTreeFile.h"
#include "BPlusStructures.h"

class BPlusTreeIterator;

class BPlusTree{
public:
    BPlusTree(string tableName, string colName, varTypes type);
    ~BPlusTree();

    void insert(data_ptr key, int rid);
    void remove(data_ptr key, int rid);
    bool has(data_ptr key);
    int count(data_ptr key);
    int lesserCount(data_ptr key);
    int greaterCount(data_ptr key);
    BPlusTreeIterator lowerBound(data_ptr key);
    BPlusTreeIterator upperBound(data_ptr key);
    vector<RID> getRIDs(data_ptr key);
    int totalCount();

    string tableName, colName;
    varTypes type;

    shared_ptr<BPlusTreeFile> treeFile = nullptr;
    shared_ptr<UnfixedRecordFile> keyFile = nullptr;

    void closeIndex();
    void deleteIndex();

    void insertIntoNonFullPage(data_ptr key, int rid, int pageID); 
    void splitChildPageOn(BPlusNode* node, int index); 
    void insertIntoOverflowPage(data_ptr key, int rid, BPlusNode* fatherPage, int x);

    void deleteFromLegalPage(data_ptr key, int rid, int pageID);
    void mergeChildPageOn(BPlusNode* node, int index);  //合并node上index和index+1号节点
    void borrowFromBackward(BPlusNode* node, int index);
    void borrowFromForward(BPlusNode* node, int index);
    void deleteFromOverflowPage(data_ptr key, int rid, BPlusNode* fatherPage, int x);

    int getCountIn(int pageID, data_ptr key);
    int getLesserCountIn(int pageID, data_ptr key);
    int getGreaterCountIn(int pageID, data_ptr key);
    BPlusTreeIterator getLowerBound(int pageID, data_ptr key);

    //debugging functions
    void printPage(int pageID);
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
    bool equals(const BPlusTreeIterator &other);

    BPlusTree* tree;
    BPlusNode* currentNode;
    BPlusOverflowPage* currentOverflowPage;
    int currentKeyPos, currentValuePos, currentCumulation;
};

#endif