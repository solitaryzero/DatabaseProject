#include "fileio/FileManager.h"

bool FileManager::openFile(const char* name, int& fileID) {
    fileID = fm->findLeftOne();
    fm->setBit(fileID, 0);
    _openFile(name, fileID);
    return true;
}
int FileManager::newType() {
    int t = tm->findLeftOne();
    tm->setBit(t, 0);
    return t;
}