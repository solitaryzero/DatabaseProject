#ifndef BPLUSTREEFILE_H
#define BPLUSTREEFILE_H

#include "../common.h"
#include "BPlusStructures.h"

class BPlusTreeFile{
public:
    BPlusTreeFile(string fileName);
    ~BPlusTreeFile();

    shared_ptr<BufPageManager> bpm = nullptr;
    int fileID, headerIndex, currentPage, currentIndex;
    BPlusHeaderPage* header;

    unsigned char* newPage(int &index);
    unsigned char* newPage();
    unsigned char* getPage(int pageID, int &index);
    unsigned char* getPage(int pageID);
    void markHeaderPageDirty();
    void markPageDirty(int index);

    void closeFile();
};

#endif