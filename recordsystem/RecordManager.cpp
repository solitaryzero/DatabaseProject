#include "RecordManager.h"

RecordManager::RecordManager(){
    this->tablePool.clear();
}

shared_ptr<TableInfo> RecordManager::newTable(string tableName){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "Table " << tableName << " already exists!\n";
        return nullptr;
    }
    ofstream tInfoOut(tableName+".tbinfo");
    tInfoOut.close();
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName, true);
    this->tablePool[tableName] = tif;
    return tif;
}

shared_ptr<TableInfo> RecordManager::newTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "Table " << tableName << " already exists!\n";
        return nullptr;
    }
    shared_ptr<TableInfo> tif = newTable(tableName);
    int ind = 0;
    for (unsigned int i=0;i<cols.size();i++){
        if ((cols[i].second == varTypes::VARCHAR_TYPE) || (cols[i].second == varTypes::CHAR_TYPE)){
            tif->addNewColumn(cols[i].first, cols[i].second, sizes[ind]);
            ind++;
        } else {
            tif->addNewColumn(cols[i].first, cols[i].second, DataOperands::getTypeSize(cols[i].second));
        }
    }
    openDataFile(tif);
    this->tablePool[tableName] = tif;
    return tif;
}

shared_ptr<TableInfo> RecordManager::openTable(string tableName){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        return this->tablePool[tableName];
    }
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName);
    openDataFile(tif);
    this->tablePool[tableName] = tif;
    return tif;
}

void RecordManager::dropTable(string tableName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "Table " << tableName << " doesn't exist!\n";
        return;
    }
    this->tablePool[tableName] = nullptr;
    unlink(string(tableName+".tbinfo").c_str());
    unlink(string(tableName+".tbdata").c_str());
}

void RecordManager::openDataFile(shared_ptr<TableInfo> p){
    p->openDataFile();
}