#include "BPlusTree.h"

BPlusTree::BPlusTree(string tableName, string colName, varTypes type){
    this->tableName = tableName;
    this->colName = colName;
    this->type = type;
    this->treeFile = make_shared<BPlusTreeFile>(this->tableName+"."+this->colName+".tree");
    this->keyFile = make_shared<UnfixedRecordFile>(this->tableName+"."+this->colName+".key");
}

BPlusTree::~BPlusTree(){
    this->closeIndex();
}

/*
====== below are interfaces ======
*/

void BPlusTree::insert(data_ptr key, int rid){
    int rootIndex;
    BPlusNode* root = (BPlusNode*)this->treeFile->getPage(this->treeFile->header->rootPageId, rootIndex);
    if (root->recCount == MAXINDEXRECPERPAGE){
        this->treeFile->markPageDirty(rootIndex);

        BPlusNode* newRoot = (BPlusNode*)this->treeFile->newPage();
        this->treeFile->header->rootPageId = newRoot->pageId;
        newRoot->nodeType = NodeType::INTERMEDIATE;
        newRoot->nextPage = 0;
        newRoot->prevPage = 0;
        newRoot->recCount = 2;

        BPlusNode* newNode = (BPlusNode*)this->treeFile->newPage();
        if (root->nodeType == NodeType::LEAF){
            root->nextPage = newNode->pageId;
            newNode->prevPage = root->pageId;
            root->recCount = MAXINDEXRECPERPAGE / 2;
            newNode->recCount = MAXINDEXRECPERPAGE - root->recCount;
            newNode->nodeType = NodeType::LEAF;

            for (int i=root->recCount;i<MAXINDEXRECPERPAGE;i++){
                newNode->data[i-root->recCount].count = root->data[i].count;
                newNode->data[i-root->recCount].keyPos = root->data[i].keyPos;
                newNode->data[i-root->recCount].value = root->data[i].value;
            }

            newRoot->data[0].count = 0;
            newRoot->data[0].keyPos = 0;
            newRoot->data[0].value = root->pageId;
            newRoot->data[1].count = 0;
            newRoot->data[1].keyPos = newNode->data[0].keyPos;
            newRoot->data[1].value = newNode->pageId;

            for (int i=0;i<root->recCount;i++){
                newRoot->data[0].count += root->data[i].count;
            }
            for (int i=0;i<newNode->recCount;i++){
                newRoot->data[1].count += newNode->data[i].count;
            }

            this->treeFile->header->lastLeaf = newNode->pageId;

            this->insertIntoNonFullPage(key, rid, newRoot->pageId);

        } else if (root->nodeType == NodeType::INTERMEDIATE) {
            root->nextPage = 0;
            newNode->prevPage = 0;
            root->recCount = MAXINDEXRECPERPAGE / 2;
            newNode->recCount = MAXINDEXRECPERPAGE - root->recCount + 1;
            newNode->nodeType = NodeType::INTERMEDIATE;
            
            //this record is actually useless
            newNode->data[0].count = 0;
            newNode->data[0].keyPos = 0;
            newNode->data[0].value = root->data[root->recCount-1].value;

            for (int i=root->recCount;i<MAXINDEXRECPERPAGE;i++){
                newNode->data[i-root->recCount+1].count = root->data[i].count;
                newNode->data[i-root->recCount+1].keyPos = root->data[i].keyPos;
                newNode->data[i-root->recCount+1].value = root->data[i].value;
            }

            newRoot->data[0].count = 0;
            newRoot->data[0].keyPos = 0;
            newRoot->data[0].value = root->pageId;
            newRoot->data[1].count = 0;
            newRoot->data[1].keyPos = newNode->data[1].keyPos;
            newRoot->data[1].value = newNode->pageId;

            for (int i=0;i<root->recCount;i++){
                newRoot->data[0].count += root->data[i].count;
            }
            for (int i=0;i<newNode->recCount;i++){
                newRoot->data[1].count += newNode->data[i].count;
            }
            this->insertIntoNonFullPage(key, rid, newRoot->pageId);
            
        } else {
            assert(false);
        }

    } else {
        insertIntoNonFullPage(key, rid, this->treeFile->header->rootPageId);
    }

    this->treeFile->header->sum++;
    this->treeFile->markHeaderPageDirty();
}

void BPlusTree::remove(data_ptr key, int rid){
    int rootIndex;
    BPlusNode* root = (BPlusNode*)this->treeFile->getPage(this->treeFile->header->rootPageId, rootIndex);
    if ((root->nodeType == NodeType::INTERMEDIATE) && (root->recCount == 2)){
        BPlusNode* child0 = (BPlusNode*)this->treeFile->getPage(root->data[0].value);
        BPlusNode* child1 = (BPlusNode*)this->treeFile->getPage(root->data[1].value);
        if ((child0->recCount == MAXINDEXRECPERPAGE/2) && (child1->recCount == MAXINDEXRECPERPAGE/2)){
            //merge page and drop empty root
            this->mergeChildPageOn(root, 0);
            assert(root->data[0].value == child0->pageId);
            this->treeFile->header->rootPageId = root->data[0].value;
        }
    }

    this->deleteFromLegalPage(key, rid, this->treeFile->header->rootPageId);
    this->treeFile->markPageDirty(rootIndex);
    this->treeFile->header->sum--;
    this->treeFile->markHeaderPageDirty();
}

bool BPlusTree::has(data_ptr key){
    return (this->count(key) > 0);
}

int BPlusTree::count(data_ptr key){
    return this->getCountIn(this->treeFile->header->rootPageId, key);
}

int BPlusTree::lesserCount(data_ptr key){
    return 0;
}

int BPlusTree::greaterCount(data_ptr key){
    return 0;
}

BPlusTreeIterator BPlusTree::lowerBound(data_ptr key){
    return this->getLowerBound(this->treeFile->header->rootPageId, key);
}

BPlusTreeIterator BPlusTree::upperBound(data_ptr key){
    BPlusTreeIterator it = this->getLowerBound(this->treeFile->header->rootPageId, key);
    if (it.available()){
        it.nextKey();
    }
    return it;
}

vector<RID> BPlusTree::getRIDs(data_ptr key){
    BPlusTreeIterator lower = this->lowerBound(key);
    BPlusTreeIterator upper = this->upperBound(key);
    vector<RID> res;
    if (lower.available()){
        assert(DataOperands::compare(this->type, key, lower.getKey()) == 0);
    }

    while (lower.available() && (!lower.equals(upper))){
        res.push_back(RID(lower.getValue()));
        lower.next();
    }
    return res;
}

int BPlusTree::totalCount(){
    return this->treeFile->header->sum;
}

/*
====== below are file operations ======
*/

void BPlusTree::closeIndex(){
    if (this->treeFile != nullptr){
        this->treeFile->closeFile();
        this->treeFile = nullptr;
    }

    if (this->keyFile != nullptr){
        this->keyFile->closeFile();
        this->keyFile = nullptr;
    }
}

void BPlusTree::deleteIndex(){
    this->closeIndex();
    string treeName = this->tableName+"."+this->colName+".tree";
    string keyName = this->tableName+"."+this->colName+".key";
    unlink(treeName.c_str());
    unlink(keyName.c_str());
}

/*
====== below are internal functions ======
*/

void BPlusTree::insertIntoNonFullPage(data_ptr key, int rid, int pageID){
    int pageIndex;
    BPlusNode* node = (BPlusNode*)(this->treeFile->getPage(pageID, pageIndex));
    if (node->nodeType == NodeType::LEAF){
        int p = 0;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) >= 0)){
            p++;
        }
        p--;

        if ((p >= 0) && (p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) == 0)){
            //duplicate key
            this->insertIntoOverflowPage(key, rid, node, p);
        } else {
            //unique key
            RID keyPos = this->keyFile->insertData(key);
            for (int i=node->recCount-1;i>p;i--){
                node->data[i+1].count = node->data[i].count;
                node->data[i+1].keyPos = node->data[i].keyPos;
                node->data[i+1].value = node->data[i].value;
            }
            node->data[p+1].count = 1;
            node->data[p+1].keyPos = keyPos.toInt();
            node->data[p+1].value = rid;
            node->recCount++;
        }
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) >= 0)){
            p++;
        }
        p--;

        BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[p].value);
        if (child->recCount == MAXINDEXRECPERPAGE){
            this->splitChildPageOn(node, p);
            if (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p+1].keyPos))) >= 0){
                p++;
            }
        }

        this->insertIntoNonFullPage(key, rid, node->data[p].value);
        node->data[p].count++;
    } else {
        assert(false);
    }
    this->treeFile->markPageDirty(pageIndex);
}

void BPlusTree::splitChildPageOn(BPlusNode* node, int index){
    int childIndex;
    BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[index].value, childIndex);
    BPlusNode* newNode = (BPlusNode*)this->treeFile->newPage();

    assert(child->recCount == MAXINDEXRECPERPAGE);
    if (child->nodeType == NodeType::LEAF){
        //modify leaf page chain
        if (child->nextPage > 0){
            int nextIndex;
            BPlusNode* next = (BPlusNode*)this->treeFile->getPage(child->nextPage, nextIndex);
            newNode->nextPage = child->nextPage;
            assert(next->prevPage == child->pageId);
            next->prevPage = newNode->pageId;
            this->treeFile->markPageDirty(nextIndex);
        }

        child->nextPage = newNode->pageId;
        newNode->prevPage = child->pageId;
        child->recCount = MAXINDEXRECPERPAGE / 2;
        newNode->recCount = MAXINDEXRECPERPAGE - child->recCount;
        newNode->nodeType = NodeType::LEAF;

        for (int i=child->recCount;i<MAXINDEXRECPERPAGE;i++){
            newNode->data[i-child->recCount].count = child->data[i].count;
            newNode->data[i-child->recCount].keyPos = child->data[i].keyPos;
            newNode->data[i-child->recCount].value = child->data[i].value;
        }

        for (int i=node->recCount-1;i>index;i--){
            node->data[i+1].count = node->data[i].count;
            node->data[i+1].keyPos = node->data[i].keyPos;
            node->data[i+1].value = node->data[i].value;
        }
        node->recCount++;

        node->data[index].count = 0;
        node->data[index+1].count = 0;
        node->data[index+1].keyPos = newNode->data[0].keyPos;
        node->data[index+1].value = newNode->pageId;

        for (int i=0;i<child->recCount;i++){
            node->data[index].count += child->data[i].count;
        }
        for (int i=0;i<newNode->recCount;i++){
            node->data[index+1].count += newNode->data[i].count;
        }

        //update lastleaf
        if (this->treeFile->header->lastLeaf == child->pageId){
            this->treeFile->header->lastLeaf = newNode->pageId;
            this->treeFile->markHeaderPageDirty();
        }

    } else if (child->nodeType == NodeType::INTERMEDIATE){
        child->nextPage = 0;
        newNode->prevPage = 0;
        child->recCount = MAXINDEXRECPERPAGE / 2;
        newNode->recCount = MAXINDEXRECPERPAGE - child->recCount + 1;
        newNode->nodeType = NodeType::INTERMEDIATE;
        
        //this record is actually useless
        newNode->data[0].count = 0;
        newNode->data[0].keyPos = 0;
        newNode->data[0].value = child->data[child->recCount-1].value;

        for (int i=child->recCount;i<MAXINDEXRECPERPAGE;i++){
            newNode->data[i-child->recCount+1].count = child->data[i].count;
            newNode->data[i-child->recCount+1].keyPos = child->data[i].keyPos;
            newNode->data[i-child->recCount+1].value = child->data[i].value;
        }

        for (int i=node->recCount-1;i>index;i--){
            node->data[i+1].count = node->data[i].count;
            node->data[i+1].keyPos = node->data[i].keyPos;
            node->data[i+1].value = node->data[i].value;
        }
        node->recCount++;

        node->data[index].count = 0;
        node->data[index+1].count = 0;
        //keyPos at data[0] is meaningless
        node->data[index+1].keyPos = newNode->data[1].keyPos;
        node->data[index+1].value = newNode->pageId;


        for (int i=0;i<child->recCount;i++){
            node->data[index].count += child->data[i].count;
        }
        for (int i=0;i<newNode->recCount;i++){
            node->data[index+1].count += newNode->data[i].count;
        }
    } else {
        assert(false);
    }

    this->treeFile->markPageDirty(childIndex);
}

void BPlusTree::insertIntoOverflowPage(data_ptr key, int rid, BPlusNode* fatherPage, int x){
    assert(fatherPage->recCount > x);
    int oldCount = fatherPage->data[x].count;
    if (oldCount == 1){
        BPlusOverflowPage* newOverflowPage = (BPlusOverflowPage*)this->treeFile->newPage();
        newOverflowPage->nodeType = NodeType::OVERFLOW;
        newOverflowPage->nextPage = 0;
        newOverflowPage->prevPage = 0;
        newOverflowPage->fatherPage = fatherPage->pageId;
        newOverflowPage->recCount = 2;
        newOverflowPage->data[0] = fatherPage->data[x].value;
        newOverflowPage->data[1] = rid;
        fatherPage->data[x].value = newOverflowPage->pageId;
        fatherPage->data[x].count = 2;
    } else if (oldCount > 1) {
        int overflowIndex;
        BPlusOverflowPage* overflowPage = (BPlusOverflowPage*)this->treeFile->getPage(fatherPage->data[x].value, overflowIndex);
        assert(overflowPage->nodeType == NodeType::OVERFLOW);
        while ((overflowPage->nextPage > 0) && (overflowPage->recCount == MAXRECPEROVERFLOWPAGE)){
            int nextID = overflowPage->nextPage;
            overflowPage = (BPlusOverflowPage*)this->treeFile->getPage(nextID, overflowIndex);
            assert(overflowPage->nodeType == NodeType::OVERFLOW);
        }

        if (overflowPage->recCount == MAXRECPEROVERFLOWPAGE){
            assert(overflowPage->nextPage == 0);
            BPlusOverflowPage* newOverflowPage = (BPlusOverflowPage*)this->treeFile->newPage();
            newOverflowPage->nodeType = NodeType::OVERFLOW;
            newOverflowPage->nextPage = 0;
            newOverflowPage->prevPage = overflowPage->pageId;
            newOverflowPage->fatherPage = fatherPage->pageId;
            newOverflowPage->recCount = 1;
            newOverflowPage->data[0] = rid;
            overflowPage->nextPage = newOverflowPage->pageId;
            fatherPage->data[x].count++;
        } else {
            overflowPage->data[overflowPage->recCount] = rid;
            overflowPage->recCount++;
            fatherPage->data[x].count++;
        }
        this->treeFile->markPageDirty(overflowIndex);
    } else {
        assert(false);
    }
}

void BPlusTree::deleteFromLegalPage(data_ptr key, int rid, int pageID){
    int pageIndex;
    BPlusNode* node = (BPlusNode*)(this->treeFile->getPage(pageID, pageIndex));
    if (node->nodeType == NodeType::LEAF){
        int p = 0;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) >= 0)){
            p++;
        }
        p--;

        assert(p >= 0);
        assert(DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) == 0);
        if (node->data[p].count > 1){
            //duplicate key
            this->deleteFromOverflowPage(key, rid, node, p);
        } else {
            //unique key
            bool res = this->keyFile->deleteData(node->data[p].keyPos);
            assert(res);
            for (int i=p;i<node->recCount;i--){
                node->data[i].count = node->data[i+1].count;
                node->data[i].keyPos = node->data[i+1].keyPos;
                node->data[i].value = node->data[i+1].value;
            }
            node->recCount--;
        }
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(RID(node->data[p].keyPos))) >= 0)){
            p++;
        }
        p--;

        BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[p].value);
        if (child->recCount <= MAXINDEXRECPERPAGE/2){
            if (p > 0){
                BPlusNode* prevChild = (BPlusNode*)this->treeFile->getPage(node->data[p-1].value);
                assert(prevChild->nodeType == child->nodeType);
                if (prevChild->recCount > MAXINDEXRECPERPAGE/2){
                    this->borrowFromBackward(node, p);
                } else {
                    this->mergeChildPageOn(node, p-1);
                }
            } else {
                BPlusNode* nextChild = (BPlusNode*)this->treeFile->getPage(node->data[p+1].value);
                assert(nextChild->nodeType == child->nodeType);
                if (nextChild->recCount > MAXINDEXRECPERPAGE/2){
                    this->borrowFromForward(node, p);
                } else {
                    this->mergeChildPageOn(node, p);
                }
            }
        }

        this->deleteFromLegalPage(key, rid, node->data[p].value);
        node->data[p].count--;

        //try to update keypos
        child = (BPlusNode*)this->treeFile->getPage(node->data[p].value);
        if (child->nodeType == NodeType::INTERMEDIATE){
            node->data[p].keyPos = child->data[1].keyPos;
        } else if (child->nodeType == NodeType::LEAF){
            node->data[p].keyPos = child->data[0].keyPos;
        } else {
            assert(false);
        }

    } else {
        assert(false);
    }
    this->treeFile->markPageDirty(pageIndex);
}

void BPlusTree::mergeChildPageOn(BPlusNode* node, int index){
    int nextIndex, childIndex;
    BPlusNode* nextChild = (BPlusNode*)this->treeFile->getPage(node->data[index+1].value, nextIndex);
    BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[index].value, childIndex);

    assert(nextChild->nodeType == child->nodeType);
    assert(nextChild->recCount + child->recCount <= MAXINDEXRECPERPAGE);

    if (child->nodeType == NodeType::LEAF){
        for (int i=0;i<nextChild->recCount;i++){
            child->data[child->recCount+i].count = nextChild->data[i].count;
            child->data[child->recCount+i].keyPos = nextChild->data[i].keyPos;
            child->data[child->recCount+i].value = nextChild->data[i].value;
        }
        child->recCount += nextChild->recCount;

        //modify leaf chain
        child->nextPage = nextChild->nextPage;
        if (nextChild->nextPage > 0){
            int nnIndex;
            BPlusNode* nnChild = (BPlusNode*)this->treeFile->getPage(nextChild->nextPage, nnIndex);
            assert(nnChild->nodeType == NodeType::LEAF);
            nnChild->prevPage = child->pageId;
            this->treeFile->markPageDirty(nnIndex);
        } else {
            assert(this->treeFile->header->lastLeaf == nextChild->pageId);
            this->treeFile->header->lastLeaf = child->pageId;
            this->treeFile->markHeaderPageDirty();
        }

    } else if (child->nodeType == NodeType::INTERMEDIATE){
        for (int i=1;i<nextChild->recCount;i++){
            child->data[child->recCount+i-1].count = nextChild->data[i].count;
            child->data[child->recCount+i-1].keyPos = nextChild->data[i].keyPos;
            child->data[child->recCount+i-1].value = nextChild->data[i].value;
        }
        child->recCount += (nextChild->recCount - 1);
    } else {
        assert(false);
    }

    node->data[index].count += node->data[index+1].count;
    for (int i=index+1;i<node->recCount;i++){
        node->data[i].count = node->data[i+1].count;
        node->data[i].keyPos = node->data[i+1].keyPos;
        node->data[i].value = node->data[i+1].value;
    }
    node->recCount--;

    this->treeFile->markPageDirty(nextIndex);
    this->treeFile->markPageDirty(childIndex);
}

void BPlusTree::borrowFromBackward(BPlusNode* node, int index){
    int prevIndex, childIndex;
    BPlusNode* prevChild = (BPlusNode*)this->treeFile->getPage(node->data[index-1].value, prevIndex);
    BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[index].value, childIndex);
    assert(prevChild->nodeType == child->nodeType);

    for (int i=child->recCount-1;i>=0;i++){
        child->data[i+1].count = child->data[i].count;
        child->data[i+1].keyPos = child->data[i].keyPos;
        child->data[i+1].value = child->data[i].value;
    }

    if (child->nodeType == NodeType::LEAF){
        child->data[0].count = prevChild->data[prevChild->recCount-1].count;
        child->data[0].keyPos = prevChild->data[prevChild->recCount-1].keyPos;
        child->data[0].value = prevChild->data[prevChild->recCount-1].value;
        //update related data in father node
        node->data[index].keyPos = child->data[0].keyPos;
        node->data[index].count += child->data[0].count;
        node->data[index-1].value -= child->data[0].count;

    } else if (child->nodeType == NodeType::INTERMEDIATE){
        child->data[1].count = prevChild->data[prevChild->recCount-1].count;
        child->data[1].keyPos = prevChild->data[prevChild->recCount-1].keyPos;
        child->data[1].value = prevChild->data[prevChild->recCount-1].value;
        //actually useless
        child->data[0].count = 0;
        child->data[0].keyPos = 0;
        child->data[0].value = prevChild->data[prevChild->recCount-2].value;
        //update related data in father node
        node->data[index].keyPos = child->data[1].keyPos;
        node->data[index].count += child->data[1].count;
        node->data[index-1].value -= child->data[1].count;

    } else {
        assert(false);
    }

    child->recCount++;
    prevChild->recCount--;
    this->treeFile->markPageDirty(prevIndex);
    this->treeFile->markPageDirty(childIndex);
}

void BPlusTree::borrowFromForward(BPlusNode* node, int index){
    int nextIndex, childIndex;
    BPlusNode* nextChild = (BPlusNode*)this->treeFile->getPage(node->data[index+1].value, nextIndex);
    BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[index].value, childIndex);
    assert(nextChild->nodeType == child->nodeType);

    if (child->nodeType == NodeType::LEAF){
        child->data[child->recCount].count = nextChild->data[0].count;
        child->data[child->recCount].keyPos = nextChild->data[0].keyPos;
        child->data[child->recCount].value = nextChild->data[0].value;

        for (int i=0;i<nextChild->recCount;i++){
            nextChild->data[i].count = nextChild->data[i+1].count;
            nextChild->data[i].keyPos = nextChild->data[i+1].keyPos;
            nextChild->data[i].value = nextChild->data[i+1].value;
        }

        //update related data in father node
        node->data[index+1].keyPos = nextChild->data[0].keyPos;
        node->data[index+1].count -= child->data[child->recCount].count;
        node->data[index].value += child->data[child->recCount].count;

    } else if (child->nodeType == NodeType::INTERMEDIATE){
        child->data[child->recCount].count = nextChild->data[1].count;
        child->data[child->recCount].keyPos = nextChild->data[1].keyPos;
        child->data[child->recCount].value = nextChild->data[1].value;

        for (int i=0;i<nextChild->recCount;i++){
            nextChild->data[i].count = nextChild->data[i+1].count;
            nextChild->data[i].keyPos = nextChild->data[i+1].keyPos;
            nextChild->data[i].value = nextChild->data[i+1].value;
        }
        nextChild->data[0].count = 0;
        nextChild->data[0].keyPos = 0;

        //update related data in father node
        node->data[index+1].keyPos = nextChild->data[1].keyPos;
        node->data[index+1].count -= child->data[child->recCount].count;
        node->data[index].value += child->data[child->recCount].count;

    } else {
        assert(false);
    }

    child->recCount++;
    nextChild->recCount--;
    this->treeFile->markPageDirty(nextIndex);
    this->treeFile->markPageDirty(childIndex);
}

void BPlusTree::deleteFromOverflowPage(data_ptr key, int rid, BPlusNode* fatherPage, int x){
    assert(fatherPage->recCount > x);
    int oldCount = fatherPage->data[x].count;
    assert(oldCount > 1);

    int overflowIndex;
    BPlusOverflowPage* overflowPage = (BPlusOverflowPage*)this->treeFile->getPage(fatherPage->data[x].value, overflowIndex);
    assert(overflowPage->nodeType == NodeType::OVERFLOW);
    int pos = -1;
    while (pos == -1){
        for (int i=0;i<overflowPage->recCount;i++){
            if (overflowPage->data[i] == rid){
                pos = i;
                break;
            }
        }
        if (overflowPage->nextPage > 0){
            overflowPage = (BPlusOverflowPage*)this->treeFile->getPage(overflowPage->nextPage, overflowIndex);
        } else {
            break;
        }
    }
    assert(pos != -1);
    for (int i=pos;i<overflowPage->recCount;i++){
        overflowPage->data[i] = overflowPage->data[i+1];
    }
    overflowPage->recCount--;

    if (overflowPage->recCount == 0){
        if (overflowPage->prevPage > 0){
            int prevIndex;
            BPlusOverflowPage* prevPage = (BPlusOverflowPage*)this->treeFile->getPage(overflowPage->prevPage, prevIndex);
            prevPage->nextPage = overflowPage->nextPage;
            this->treeFile->markPageDirty(prevIndex);
        }

        if (overflowPage->nextPage > 0){
            int nextIndex;
            BPlusOverflowPage* nextPage = (BPlusOverflowPage*)this->treeFile->getPage(overflowPage->nextPage, nextIndex);
            nextPage->prevPage = overflowPage->prevPage;
            this->treeFile->markPageDirty(nextIndex);
        }

        if (fatherPage->data[x].value == overflowPage->pageId){
            fatherPage->data[x].value = overflowPage->nextPage;
        }
    }

    fatherPage->data[x].count--;
    if (fatherPage->data[x].count == 1){
        overflowPage = (BPlusOverflowPage*)this->treeFile->getPage(fatherPage->data[x].value);
        assert(overflowPage->nodeType == NodeType::OVERFLOW);
        assert(overflowPage->recCount == 1);
        assert(overflowPage->prevPage == 0);
        assert(overflowPage->nextPage == 0);
        fatherPage->data[x].value = overflowPage->data[0];
    }

    this->treeFile->markPageDirty(overflowIndex);
}

int BPlusTree::getCountIn(int pageID, data_ptr key){
    BPlusNode* node = (BPlusNode*)this->treeFile->getPage(pageID);
    if (node->nodeType == NodeType::LEAF){
        for (int i=0;i<node->recCount;i++){
            if (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[i].keyPos)) == 0){
                return node->data[i].count;
            }
        }
        return -1;
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;
        return this->getCountIn(node->data[p].value, key);
    } else {
        return -1;
    }
}

int BPlusTree::getLesserCountIn(int pageID, data_ptr key){
    BPlusNode* node = (BPlusNode*)this->treeFile->getPage(pageID);
    int res = 0;
    if (node->nodeType == NodeType::LEAF){
        for (int i=0;i<node->recCount;i++){
            if (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[i].keyPos)) <= 0){
                return res;
            }
            res += node->data[i].count;
        }
        return res;
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;
        for (int i=0;i<p;i++){
            res += node->data[i].count;
        }
        return res+this->getLesserCountIn(node->data[p].value, key);
    } else {
        return -1;
    }
}

int BPlusTree::getGreaterCountIn(int pageID, data_ptr key){
    BPlusNode* node = (BPlusNode*)this->treeFile->getPage(pageID);
    int res = 0;
    if (node->nodeType == NodeType::LEAF){
        for (int i=node->recCount-1;i>=0;i--){
            if (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[i].keyPos)) >= 0){
                return res;
            }
            res += node->data[i].count;
        }
        return res;
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;
        for (int i=p+1;i<node->recCount;i++){
            res += node->data[i].count;
        }
        return res+this->getGreaterCountIn(node->data[p].value, key);
    } else {
        return -1;
    }
}

BPlusTreeIterator BPlusTree::getLowerBound(int pageID, data_ptr key){
    BPlusNode* node = (BPlusNode*)this->treeFile->getPage(pageID);
    if (node->nodeType == NodeType::LEAF){
        for (int i=0;i<node->recCount;i++){
            if (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[i].keyPos)) == 0){
                return BPlusTreeIterator(this, node, i, 0);
            }
        }
        return BPlusTreeIterator(this);
    } else if (node->nodeType == NodeType::INTERMEDIATE){
        int p = 1;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;
        return this->getLowerBound(node->data[p].value, key);
    } else {
        return BPlusTreeIterator(this);
    }
}

/*
====== BPlusTreeIterator below: ======
*/

BPlusTreeIterator::BPlusTreeIterator(BPlusTree* bt){
    this->tree = bt;
    this->currentNode = nullptr;
    this->currentKeyPos = -1;
    this->currentValuePos = -1;
}

BPlusTreeIterator::BPlusTreeIterator(BPlusTree* bt, BPlusNode* bn, int keyn, int valn){
    this->tree = bt;
    this->currentNode = bn;
    this->currentKeyPos = keyn;
    this->currentValuePos = valn;
}

data_ptr BPlusTreeIterator::getKey(){
    assert((this->tree != nullptr) && (this->currentNode != nullptr));
    assert((this->currentKeyPos >= 0) && (this->currentKeyPos < this->currentNode->recCount));
    return this->tree->keyFile->getData(RID(this->currentNode->data[this->currentKeyPos].keyPos));
}

int BPlusTreeIterator::getValue(){
    assert((this->tree != nullptr) && (this->currentNode != nullptr));
    assert((this->currentKeyPos >= 0) && (this->currentKeyPos < this->currentNode->recCount));
    int c = this->currentNode->data[this->currentKeyPos].count;
    assert((this->currentValuePos >= 0) && (this->currentValuePos < c));

    if (c == 1){
        return this->currentNode->data[this->currentKeyPos].value;
    }

    if (this->currentValuePos == 0){
        this->currentCumulation = 0;
        this->currentOverflowPage = (BPlusOverflowPage*)(this->tree->treeFile->getPage(this->currentNode->data[this->currentKeyPos].value));
    }

    assert(this->currentOverflowPage != nullptr);
    while (this->currentCumulation+this->currentOverflowPage->recCount < this->currentValuePos){
        this->currentCumulation += this->currentOverflowPage->recCount;
        this->currentOverflowPage = (BPlusOverflowPage*)(this->tree->treeFile->getPage(this->currentOverflowPage->nextPage));
    }
    while (this->currentCumulation > this->currentValuePos){
        this->currentOverflowPage = (BPlusOverflowPage*)(this->tree->treeFile->getPage(this->currentOverflowPage->prevPage));
        this->currentCumulation -= this->currentOverflowPage->recCount;
    }

    return (this->currentOverflowPage->data[this->currentValuePos-this->currentCumulation]);
}

void BPlusTreeIterator::next(){
    assert(this->currentNode != nullptr);
    assert(this->available());
    int c = this->currentNode->data[this->currentKeyPos].count;
    assert(this->currentValuePos < c);
    if (this->currentValuePos < c-1){
        this->currentValuePos++;
    } else {
        this->nextKey();
    }
}

void BPlusTreeIterator::nextKey(){
    this->currentCumulation = 0;
    this->currentOverflowPage = nullptr;
    int c = this->currentNode->recCount;
    if (this->currentKeyPos < c-1){
        this->currentKeyPos++;
    } else {
        int pid = this->currentNode->nextPage;
        if (pid <= 0){
            this->currentNode = nullptr;
            this->currentKeyPos = 0;
            this->currentValuePos = 0;
        } else {
            this->currentNode = (BPlusNode*)this->tree->treeFile->getPage(pid);
            this->currentKeyPos = 0;
            this->currentValuePos = 0;
        }
    }
}

void BPlusTreeIterator::previous(){
    assert(this->currentNode != nullptr);
    assert(this->available());
    assert(this->currentValuePos >= 0);
    if (this->currentValuePos > 0){
        this->currentValuePos--;
    } else {
        this->previousKey();
    }
}

void BPlusTreeIterator::previousKey(){
    this->currentCumulation = 0;
    this->currentOverflowPage = nullptr;
    if (this->currentKeyPos > 0){
        this->currentKeyPos--;
    } else {
        int pid = this->currentNode->prevPage;
        if (pid <= 0){
            this->currentNode = nullptr;
            this->currentKeyPos = 0;
            this->currentValuePos = 0;
        } else {
            this->currentNode = (BPlusNode*)this->tree->treeFile->getPage(pid);
            this->currentKeyPos = this->currentNode->recCount-1;
            this->currentValuePos = this->currentNode->data[this->currentKeyPos].count-1;
        }
    }
}

bool BPlusTreeIterator::available(){
    return (this->currentNode != nullptr);
}

void BPlusTreeIterator::setToBegin(){
    this->currentKeyPos = 0;
    this->currentValuePos = 0;
    this->currentCumulation = 0;
    this->currentOverflowPage = nullptr;
    this->currentNode = (BPlusNode*)this->tree->treeFile->getPage(this->tree->treeFile->header->rootPageId);
}

bool BPlusTreeIterator::equals(const BPlusTreeIterator &other){
    if (this->currentNode == nullptr){
        if (other.currentNode == nullptr){
            return true;
        }
        return false;
    }

    return ((this->currentKeyPos == other.currentKeyPos) && (this->currentValuePos == other.currentValuePos) && (this->currentNode == other.currentNode));
}