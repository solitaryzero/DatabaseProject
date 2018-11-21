#include "UnfixedRecordFile.h"

UnfixedRecordFile::UnfixedRecordFile(string filename){
    FileManager *fm = new FileManager();
    if (!fm->openFile(filename.c_str(), this->fileID)){
        fm->createFile(filename.c_str());
        bool res = fm->openFile(filename.c_str(), this->fileID);
        assert(res == true);
    }

    this->bpm = make_shared<BufPageManager>(fm);
    this->header = (UnfixedRecordFileHeader*)this->bpm->getPage(this->fileID, 0, this->headerIndex);
    if (this->header->isValid == 0){
        this->header->isValid = 1;
        this->header->pageNum = 1;
        this->bpm->markDirty(this->headerIndex);
    }
}

UnfixedRecordFile::~UnfixedRecordFile(){
    if (this->bpm != nullptr){
        this->bpm->close();
    }
}

RID UnfixedRecordFile::insertData(data_ptr dat){
    assert(dat->size() <= PAGE_SIZE-8-8);
    int index;
    this->currentPage = header->pageNum;
    unsigned char *page;
    int lastFreeOffset, slotNum, availableSpace;
    while (true){
        page = (unsigned char*)(this->bpm->getPage(this->fileID, this->currentPage, index));
        lastFreeOffset = *(int*)(page+PAGE_SIZE-4);
        slotNum = *(int*)(page+PAGE_SIZE-8);
        availableSpace = PAGE_SIZE-8-lastFreeOffset-8*slotNum;
        if (availableSpace-8 >= (int)dat->size()){
            break;
        }
        this->currentPage++;
    }
    
    memcpy(page+lastFreeOffset, dat->data(), dat->size());
    *(int*)(page+PAGE_SIZE-8-8*slotNum-4) = lastFreeOffset;
    *(int*)(page+PAGE_SIZE-8-8*slotNum-8) = dat->size();
    slotNum++;
    lastFreeOffset += dat->size();
    *(int*)(page+PAGE_SIZE-4) = lastFreeOffset;
    *(int*)(page+PAGE_SIZE-8) = slotNum;

    this->bpm->markDirty(index);
    this->currentSlot = slotNum-1;
    if (this->header->pageNum < this->currentPage){
        this->bpm->markDirty(this->headerIndex);
        this->header->pageNum = this->currentPage;
    }
    return RID(this->currentPage, this->currentSlot);
}

bool UnfixedRecordFile::deleteData(RID target){
    int pn = target.pagenum;
    int sn = target.slotnum;
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, pn, index));
    int slotNum = *(int*)(page+PAGE_SIZE-8);
    if (slotNum < sn){
        perror("delete failed");
        return false;
    }
    *(int*)(page+PAGE_SIZE-8-8*sn-8) = -1;
    this->currentPage = pn;
    this->currentSlot = sn;
    this->bpm->markDirty(index);
    return true;
}

data_ptr UnfixedRecordFile::getData(RID target){
    int pn = target.pagenum;
    int sn = target.slotnum;
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, pn, index));
    int slotNum = *(int*)(page+PAGE_SIZE-8);
    if (slotNum < sn){
        return nullptr;
    }
    int size = *(int*)(page+PAGE_SIZE-8-8*sn-8);
    if (size == -1){
        return nullptr;
    }
    int offset = *(int*)(page+PAGE_SIZE-8-8*sn-4);
    data_ptr p = DataContainer::genDataContainer(size);
    memcpy(p->data(), page+offset, size);
    this->currentPage = pn;
    this->currentSlot = sn;
    return p;
}

RID UnfixedRecordFile::updateData(RID target, data_ptr dat){
    bool res = this->deleteData(target);
    assert(res == true);
    return (this->insertData(dat));
}

RID UnfixedRecordFile::getCurrentRID(){
    return RID(this->currentPage, this->currentSlot);
}

data_ptr UnfixedRecordFile::firstData(){
    if (this->bpm == nullptr) return nullptr;
    this->currentPage = 1;
    this->currentSlot = 0;
    data_ptr res = getData(RID(this->currentPage, this->currentSlot));
    if (res != nullptr){
        return res;
    }
    return nextData();
}

data_ptr UnfixedRecordFile::nextData(){
    if (this->currentPage > this->header->pageNum){
        return nullptr;
    }
    this->currentSlot++;
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, this->currentPage, index));
    int slotNum;
    slotNum = *(int*)(page+PAGE_SIZE-8);
    while (this->currentPage <= this->header->pageNum){
        while (this->currentSlot >= slotNum){
            this->currentPage++;
            this->currentSlot = 0;
            page = (unsigned char*)(this->bpm->getPage(this->fileID, this->currentPage, index));
            slotNum = *(int*)(page+PAGE_SIZE-8);
            if (this->currentPage > this->header->pageNum){
                return nullptr;
            }
        }
        data_ptr res = getData(RID(this->currentPage, this->currentSlot));
        if (res != nullptr){
            return res;
        }
        this->currentSlot++;
    }

    return nullptr;
}

void UnfixedRecordFile::closeFile(){
    assert(this->bpm != nullptr);
    this->bpm->close();
    this->bpm = nullptr;
}