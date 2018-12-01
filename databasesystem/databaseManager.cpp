#include "databaseManager.h"

DatabaseManager::DatabaseManager(){
    this->tablePool.clear();
    char buf[200];
    this->pathBase = string(getcwd(buf, 200));
}

DatabaseManager::~DatabaseManager(){
    this->tablePool.clear();
}

void DatabaseManager::switchDatabase(string dbName){
    if (this->databaseName != ""){
        this->tablePool.clear();
    }
    this->databaseName = dbName;
    string fullPath = this->pathBase+"/"+this->databaseName;
    chdir(fullPath.c_str());

    DIR *dir = opendir(fullPath.c_str());
    if (dir == nullptr){
        cout << "open directory failed\n";
        return;
    }

    struct dirent *filename;
    while((filename = readdir(dir)) != nullptr){
        string fname(filename->d_name);
        string suffix = ".tbinfo";
        if (fname.compare(fname.length() - suffix.length(), suffix.length(), suffix) == 0){
            this->openTable(fname.substr(0, fname.length()-suffix.length()));
        }
    } 
}

shared_ptr<TableInfo> DatabaseManager::createTable(string tableName){
    if (this->databaseName == ""){
        cout << "Not using any database now!\n";
        return nullptr;
    }
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

shared_ptr<TableInfo> DatabaseManager::createTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes, vector<int> useIndex){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "Table " << tableName << " already exists!\n";
        return nullptr;
    }
    shared_ptr<TableInfo> tif = createTable(tableName);
    int ind = 0;
    for (unsigned int i=0;i<cols.size();i++){
        if ((cols[i].second == varTypes::VARCHAR_TYPE) || (cols[i].second == varTypes::CHAR_TYPE)){
            tif->addNewColumn(cols[i].first, cols[i].second, sizes[ind], useIndex[i]);
            ind++;
        } else {
            tif->addNewColumn(cols[i].first, cols[i].second, DataOperands::getTypeSize(cols[i].second), useIndex[i]);
        }
    }
    openDataFile(tif);
    this->tablePool[tableName] = tif;
    return tif;
}

shared_ptr<TableInfo> DatabaseManager::openTable(string tableName){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        return this->tablePool[tableName];
    }
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName);
    openDataFile(tif);
    this->tablePool[tableName] = tif;
    return tif;
}

void DatabaseManager::dropTable(string tableName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "Table " << tableName << " doesn't exist!\n";
        return;
    }
    this->tablePool[tableName] = nullptr;
    unlink(string(tableName+".tbinfo").c_str());
    unlink(string(tableName+".tbdata").c_str());
}

void DatabaseManager::openDataFile(shared_ptr<TableInfo> p){
    p->openDataFile();
}