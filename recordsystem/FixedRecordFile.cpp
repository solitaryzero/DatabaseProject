#include "FixedRecordFile.h"

FixedRecordFile::FixedRecordFile(string filename){
    FileManager *fm = new FileManager();
    if (!fm->openFile(filename.c_str(), this->fileID)){
        fm->createFile(filename.c_str());
        bool res = fm->openFile(filename.c_str(), this->fileID);
        assert(res == true);
    }

    this->bpm = make_shared<BufPageManager>(fm);
    int index;
    this->header = (FixedRecordFileHeader*)this->bpm->getPage(this->fileID, 0, index);
    if (this->header->isValid == 0){
        this->header->isValid = 1;
        this->header->pageNum = 1;
    }
}

FixedRecordFile::~FixedRecordFile(){
    if (this->bpm != nullptr){
        this->bpm->close();
    }
}

RID FixedRecordFile::insertData(data_ptr dat){
    return RID();
}

bool FixedRecordFile::deleteData(RID target){
    return false;
}

data_ptr FixedRecordFile::getData(RID target){
    return nullptr;
}

RID FixedRecordFile::updateData(RID target, data_ptr dat){
    return RID();
}