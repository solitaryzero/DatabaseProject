#include "FixedRecordFile.h"

FixedRecordFile::FixedRecordFile(string filename){
    FileManager *fm = new FileManager();
    if (!fm->openFile(filename.c_str(), this->fileID)){
        fm->createFile(filename.c_str());
        bool res = fm->openFile(filename.c_str(), this->fileID);
        assert(res == true);
    }

    this->bpm = make_shared<BufPageManager>(fm);
    this->header = (FixedRecordFileHeader*)this->bpm->getPage(this->fileID, 0, this->headerIndex);
    if (this->header->isValid == 0){
        this->header->isValid = 1;
        this->header->pageNum = 1;
        this->bpm->markDirty(this->headerIndex);

        unsigned char *page;
        int index;
        page = (unsigned char*)(this->bpm->getPage(this->fileID, 1, index));
        memset(page, 0, PAGE_SIZE);
        this->bpm->markDirty(index);
    }
}

FixedRecordFile::~FixedRecordFile(){
    if (this->bpm != nullptr){
        this->bpm->close();
    }
}

RID FixedRecordFile::insertData(data_ptr dat){
    assert((int)dat->size() <= PAGE_SIZE-this->dataOffset);
    assert((int)dat->size() == this->header->recordLength);
    int index;
    unsigned char *page;
    FixedRecordPageHeader *pheader;
    this->currentPage = 1;
    while (true){
        page = (unsigned char*)(this->bpm->getPage(this->fileID, this->currentPage, index));

        if (this->currentPage > this->header->pageNum){
            //I shouldn't have believed that stupid filemanager :(
            memset(page, 0, PAGE_SIZE);
        }

        pheader = (FixedRecordPageHeader*)page;
        if (pheader->usedSlot >= this->header->availableRecordPerPage){
            this->currentPage++;
        } else {
            break;
        }
    }

    assert(pheader->usedSlot < this->header->availableRecordPerPage);
    int freeIndex = getNextZero(page+sizeof(FixedRecordPageHeader), this->bitmapSize);
    assert(freeIndex != -1);
    memcpy(page+this->dataOffset+freeIndex*this->header->recordLength, dat->data(), dat->size());

    pheader->usedSlot++;
    setBitAt(page+sizeof(FixedRecordPageHeader), freeIndex, 1);
    this->bpm->markDirty(index);
    this->currentSlot = freeIndex;

    if (this->header->pageNum < this->currentPage){
        this->header->pageNum = this->currentPage;
        this->bpm->markDirty(this->headerIndex);
    }

    return RID(this->currentPage, this->currentSlot);
}

bool FixedRecordFile::deleteData(RID target){
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, target.pagenum, index));
    FixedRecordPageHeader *pheader = (FixedRecordPageHeader*)page;
    int check = getBitAt(page+sizeof(FixedRecordPageHeader), target.slotnum);
    if (check == 0){
        perror("delete failed!\n");
        return false;
    }

    setBitAt(page+sizeof(FixedRecordPageHeader), target.slotnum, 0);
    pheader->usedSlot--;
    this->currentPage = target.pagenum;
    this->currentSlot = target.slotnum;
    this->bpm->markDirty(index);
    return true;
}

data_ptr FixedRecordFile::getData(RID target){
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, target.pagenum, index));
    if (target.slotnum >= this->header->availableRecordPerPage){
        return nullptr;
    }
    int check = getBitAt(page+sizeof(FixedRecordPageHeader), target.slotnum);
    if (check == 0){
        return nullptr;
    }

    data_ptr res = DataContainer::genDataContainer(this->header->recordLength);
    unsigned char *pos = page+this->dataOffset+target.slotnum*this->header->recordLength;
    memcpy(res->data(), pos, this->header->recordLength);
    this->currentPage = target.pagenum;
    this->currentSlot = target.slotnum;
    return res;
}

RID FixedRecordFile::updateData(RID target, data_ptr dat){
    assert((int)dat->size() == this->header->recordLength);
    int index;
    unsigned char *page = (unsigned char*)(this->bpm->getPage(this->fileID, target.pagenum, index));
    int check = getBitAt(page+sizeof(FixedRecordPageHeader), target.slotnum);
    if (check == 0){
        perror("update failed!\n");
        return RID();
    }

    unsigned char *pos = page+this->dataOffset+target.slotnum*this->header->recordLength;
    memcpy(pos, dat->data(), this->header->recordLength);
    this->currentPage = target.pagenum;
    this->currentSlot = target.slotnum;
    this->bpm->markDirty(index);
    return target;
}

RID FixedRecordFile::getCurrentRID(){
    return RID(this->currentPage, this->currentSlot);
}

data_ptr FixedRecordFile::firstData(){
    if (this->bpm == nullptr) return nullptr;
    this->currentPage = 1;
    this->currentSlot = 0;
    data_ptr res = getData(RID(this->currentPage, this->currentSlot));
    if (res != nullptr){
        return res;
    }
    return nextData();
}

data_ptr FixedRecordFile::nextData(){
    if (this->currentPage > this->header->pageNum){
        return nullptr;
    }
    this->currentSlot++;
    while (this->currentPage <= this->header->pageNum){
        while (this->currentSlot >= this->header->availableRecordPerPage){
            this->currentPage++;
            this->currentSlot = 0;
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

void FixedRecordFile::closeFile(){
    assert(this->bpm != nullptr);
    this->bpm->close();
    this->bpm = nullptr;
}

void FixedRecordFile::setRecordLength(int len){
    this->header->recordLength = len;
    calculateRecordPerPage();
}

void FixedRecordFile::calculateRecordPerPage(){
    assert(this->header != nullptr);
    assert(this->header->recordLength != 0);
    this->header->availableRecordPerPage = (PAGE_SIZE-this->dataOffset)/this->header->recordLength;
    this->bpm->markDirty(this->headerIndex);
}

int FixedRecordFile::getNextZero(unsigned char *start, int upperBound){
    for (int i=0;i<upperBound;i++){
        if (getBitAt(start, i) == 0) {
            return i;
        }
    }
    return -1;
}

int FixedRecordFile::getBitAt(unsigned char *start, int place){
    start += place/8;
    unsigned char d = (*start) >> (7-(place%8));
    return (int)(d & 1);
}

void FixedRecordFile::setBitAt(unsigned char *start, int place, int target){
    start += place/8;
    if (target == 0){
        *start = ((*start) & (~(1 << (7-(place%8)))));
    } else {
        *start = ((*start) | (1 << (7-(place%8))));
    }
}