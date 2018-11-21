#include "BPlusTree.h"

BPlusTree::BPlusTree(string tableName, string colName){
    this->tableName = tableName;
    this->colName = colName;
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