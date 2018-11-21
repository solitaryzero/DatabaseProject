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
}

void BPlusTree::remove(data_ptr key, int rid){
    
}

bool BPlusTree::has(data_ptr key){
    return false;
}

int BPlusTree::count(data_ptr key){
    return 0;
}

int BPlusTree::greaterCount(data_ptr key){
    return 0;
}

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

void BPlusTree::insertIntoNonFullPage(data_ptr key, int rid, int pageID){
    int pageIndex;
    BPlusNode* node = (BPlusNode*)(this->treeFile->getPage(pageID, pageIndex));
    if (node->nodeType == NodeType::LEAF){
        int p = 0;
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;

        if ((p >= 0) && (p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) == 0)){
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
        while ((p < node->recCount) && (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p].keyPos)) >= 0)){
            p++;
        }
        p--;

        BPlusNode* child = (BPlusNode*)this->treeFile->getPage(node->data[p].value);
        if (child->recCount == MAXINDEXRECPERPAGE){
            this->splitChildPageOn(node, p);
            if (DataOperands::compare(this->type, key, this->keyFile->getData(node->data[p+1].keyPos)) >= 0){
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