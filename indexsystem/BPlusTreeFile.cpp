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
    }
}

unsigned char* BPlusTreeFile::newPage(){
    this->header->pageCount++;
    int index;
    return (unsigned char*)this->bpm->getPage(this->fileID, this->header->pageCount, index);
}

unsigned char* BPlusTreeFile::getPage(int pageID){
    int index;
    return (unsigned char*)this->bpm->getPage(this->fileID, pageID, index);
}

void BPlusTreeFile::markDirty(int pageID){
    this->bpm->markDirty(pageID);
}