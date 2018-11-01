#include "RecordManager.h"

void RecordManager::newTable(string tableName){
    ofstream tInfoOut(tableName+".tbinfo");
    tInfoOut.close();
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName, true);
    this->tinfo = tif;
    this->infoFileName = tableName+".tbinfo";
    this->dataFileName = tableName+".tbdata";
}

void RecordManager::newTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes){
    newTable(tableName);
    int ind = 0;
    for (unsigned int i=0;i<cols.size();i++){
        if ((cols[i].second == varTypes::VARCHAR_TYPE) || (cols[i].second == varTypes::CHAR_TYPE)){
            this->tinfo->addNewColumn(cols[i].first, cols[i].second, sizes[ind]);
            ind++;
        } else {
            this->tinfo->addNewColumn(cols[i].first, cols[i].second, DataOperands::getTypeSize(cols[i].second));
        }
    }
    openDataFile();
}

void RecordManager::openTable(string tableName){
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName);
    this->tinfo = tif;
    this->infoFileName = tableName+".tbinfo";
    this->dataFileName = tableName+".tbdata";
    openDataFile();
}

void RecordManager::dropTable(string tableName){
    if (this->tinfo != nullptr){
        this->tinfo->writeBack();
    }
    this->tinfo = nullptr;
    this->fixedFile = nullptr;
    this->unfixedFile = nullptr;
    unlink(string(tableName+".tbinfo").c_str());
    unlink(string(tableName+".tbdata").c_str());
}

void RecordManager::openDataFile(){
    this->fixedFile = nullptr;
    this->unfixedFile = nullptr;
    if (this->tinfo->unfixedColNumbers == 0){
        this->fixedFile = make_shared<FixedRecordFile>(this->dataFileName);
        this->fixedFile->setRecordLength(this->tinfo->getFixedRecordLength());
        this->isFixed = true;
    } else {
        this->unfixedFile = make_shared<UnfixedRecordFile>(this->dataFileName);
        this->isFixed = false;
    }
    this->tinfo->genConverter();
}