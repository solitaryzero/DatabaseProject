#include "BPlusTreeFile.h"

BPlusTreeFile::BPlusTreeFile(string fileName){
    FileManager *fm = new FileManager();
    if (!fm->openFile(fileName.c_str(), this->fileID)){
        fm->createFile(fileName.c_str());
        bool res = fm->openFile(fileName.c_str(), this->fileID);
        assert(res == true);
    }

    this->bpm = make_shared<BufPageManager>(fm);
    this->header = (BPlusHeaderPage*)this->bpm->getPage(this->fileID, 0, this->headerIndex);
    if (this->header->rootPageId == 0){
        this->header->rootPageId = 1;
        this->header->pageCount = 1;
        this->header->firstLeaf = 1;
        this->header->lastLeaf = 1;
        this->header->sum = 0;
        this->bpm->markDirty(this->headerIndex);

        int index;
        BPlusNode* root = (BPlusNode*)getPage(1, index);
        root->nextPage = 0;
        root->prevPage = 0;
        root->nodeType = NodeType::LEAF;
        root->pageId = 1;
        root->recCount = 0;
        this->markPageDirty(index);
    }
}

BPlusTreeFile::~BPlusTreeFile(){
    this->closeFile();
}

unsigned char* BPlusTreeFile::newPage(int &index){
    this->header->pageCount++;
    unsigned char* res = (unsigned char*)this->bpm->getPage(this->fileID, this->header->pageCount, index);
    ((BPlusNode*)res)->pageId = this->header->pageCount;
    this->markPageDirty(index);
    this->markHeaderPageDirty();
    return res;
}

unsigned char* BPlusTreeFile::newPage(){
    int index;
    this->header->pageCount++;
    unsigned char* res = (unsigned char*)this->bpm->getPage(this->fileID, this->header->pageCount, index);
    ((BPlusNode*)res)->pageId = this->header->pageCount;
    this->markPageDirty(index);
    this->markHeaderPageDirty();
    return res;
}

unsigned char* BPlusTreeFile::getPage(int pageID, int &index){
    assert(pageID <= this->header->pageCount);
    return (unsigned char*)this->bpm->getPage(this->fileID, pageID, index);
}

unsigned char* BPlusTreeFile::getPage(int pageID){
    assert(pageID <= this->header->pageCount);
    int index;
    return (unsigned char*)this->bpm->getPage(this->fileID, pageID, index);
}

void BPlusTreeFile::markHeaderPageDirty(){
    this->bpm->markDirty(this->headerIndex);
}

void BPlusTreeFile::markPageDirty(int index){
    this->bpm->markDirty(index);
}

void BPlusTreeFile::closeFile(){
    if (this->bpm != nullptr){
        this->bpm->close();
        this->bpm = nullptr;
    }
}