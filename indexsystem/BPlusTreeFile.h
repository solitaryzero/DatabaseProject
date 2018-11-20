#ifndef BPLUSTREEFILE_H
#define BPLUSTREEFILE_H

#include "../common.h"
#include "BPlusStructures.h"

class BPlusTreeFile{
public:
    BPlusTreeFile(string fileName);

    shared_ptr<BufPageManager> bpm = nullptr;
    int fileID, headerIndex, currentPage;
    BPlusHeaderPage* header;

    unsigned char* newPage();
    unsigned char* getPage(int pageID);
    void markDirty(int pageID);
};

#endif