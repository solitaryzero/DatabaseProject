#ifndef BPLUSSTRUCTURES_H
#define BPLUSSTRUCTURES_H

#include "../common.h"

enum NodeType {UNDEFINED = 0, INTERMEDIATE = 1, LEAF = 2, OVERFLOW = 3};

struct IndexRecord{
    int keyPos;
    int value;  
    //for leaf page: count>1 -> overflowpage id; count==1 -> RID
    //for intermediate page: pointer right to this page
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
    int pageCount;
    int firstLeaf;
    int lastLeaf;
    int sum;
};

#endif