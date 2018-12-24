#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(){
    this->tablePool.clear();
    char buf[200];
    this->pathBase = string(getcwd(buf, 200))+"/data/";
}

DatabaseManager::~DatabaseManager(){
    this->tablePool.clear();
}

void DatabaseManager::showDatabases(){
    string fullPath = this->pathBase;

    DIR *dir = opendir(fullPath.c_str());
    if (dir == nullptr){
        cout << "[Error] open directory failed\n";
        return;
    }

    cout << "[Info] Databases\n";
    //cout << "================\n";
    struct dirent *filename;
    int cnt = 0;
    while((filename = readdir(dir)) != nullptr){
        string fname(filename->d_name);
        if ((filename->d_type == DT_DIR) && (fname != ".") && (fname != "..")){
            cout << fname << "\n";
            cnt++;
        }
    }
    cout << "[Info] " << cnt << " databases in total.\n";
    //cout << "================\n";
}

void DatabaseManager::switchDatabase(string dbName){
    if ((dbName == ".") || (dbName == "..")){
        return;
    }

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
    while ((filename = readdir(dir)) != nullptr){
        string fname(filename->d_name);
        string suffix = ".tbinfo";
        if (fname.length() <= suffix.length()){
            continue;
        }
        if (fname.compare(fname.length() - suffix.length(), suffix.length(), suffix) == 0){
            this->openTable(fname.substr(0, fname.length()-suffix.length()));
        }
    } 

    for (auto x : this->tablePool){
        shared_ptr<TableInfo> ti = x.second;
        for (int i=0;i<ti->colNumbers;i++){
            shared_ptr<ColumnInfo> ci = ti->colInfos[i];
            if (ci->hasForeign){
                string dt = ci->foreignTableName;
                string dc = ci->foreignColumnName;
                assert(this->tablePool.find(dt) != this->tablePool.end());
                assert(this->tablePool[dt]->colInfoMapping.find(dc) != this->tablePool[dt]->colInfoMapping.end());
                this->tablePool[dt]->colInfoMapping[dc]->referedBy.push_back({ti->tableName, ci->columnName});
            }
        }
    }
}

void DatabaseManager::createDatabase(string dbName){
    string fullPath = this->pathBase+"/"+dbName;
    if (access(fullPath.c_str(), F_OK) == 0){
        cout << "[Error] Database " << dbName << " already exists!\n";
        return;
    }

    mkdir(fullPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void DatabaseManager::dropDatabase(string dbName){
    string fullPath = this->pathBase+"/"+dbName;
    if (access(fullPath.c_str(), F_OK) != 0){
        cout << "[Error] Database" << dbName << "doesn't exist!\n";
        return;
    }

    if (this->databaseName == dbName){
        this->databaseName == "";
    }
    rmdir(fullPath.c_str());
}

void DatabaseManager::showTables(){
    if (this->databaseName == ""){
        cout << "[Error] Not using any database!\n";
        return;
    }

    cout << "[Info] " << this->tablePool.size() << " tables in " << this->databaseName << ":\n";
    //cout << "================\n";

    if (this->tablePool.size() == 0){
        //cout << "[Info] No tables found.\n";
        return;
    }

    auto it = this->tablePool.begin();
    while (it != this->tablePool.end()){
        cout << it->first << "\n";
        it++;
    }

    //cout << "================\n";
}

shared_ptr<TableInfo> DatabaseManager::createTable(string tableName){
    if (this->databaseName == ""){
        cout << "[Error] Not using any database!\n";
        return nullptr;
    }
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "[Error] Table " << tableName << " already exists!\n";
        return nullptr;
    }

    ofstream tInfoOut(tableName+".tbinfo");
    tInfoOut.close();
    shared_ptr<TableInfo> tif = make_shared<TableInfo>(tableName, true);
    this->tablePool[tableName] = tif;
    return tif;
}

shared_ptr<TableInfo> DatabaseManager::createTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "[Error] Table " << tableName << " already exists!\n";
        return nullptr;
    }
    shared_ptr<TableInfo> tif = createTable(tableName);
    for (unsigned int i=0;i<cols.size();i++){
        /*
        if ((cols[i].second == varTypes::VARCHAR_TYPE) || (cols[i].second == varTypes::CHAR_TYPE)){
            tif->addNewColumn(cols[i].first, cols[i].second, sizes[i]);
        } else {
            tif->addNewColumn(cols[i].first, cols[i].second, DataOperands::getTypeSize(cols[i].second));
        }
        */
        tif->addNewColumn(cols[i].first, cols[i].second, sizes[i]);
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
        cout << "[Error] Table " << tableName << " doesn't exist!\n";
        return;
    }

    shared_ptr<TableInfo> ti = this->tablePool[tableName];
    //check if used as foreign key
    for (int i=0;i<ti->colNumbers;i++){
        if (ti->colInfos[i]->referedBy.size() > 0){
            cout << "[Error] Column " << ti->colInfos[i]->columnName << " refered by (" 
            << ti->colInfos[i]->referedBy[0].first << "." << ti->colInfos[i]->referedBy[0].second << "), abort.\n";
            return;
        }
    }

    //remove foreign key dependencies
    for (int i=0;i<ti->colNumbers;i++){
        if (ti->colInfos[i]->hasForeign){
            string dtName = ti->colInfos[i]->foreignTableName;
            string dcName = ti->colInfos[i]->foreignColumnName;
            assert((dtName != "") && (dcName != ""));
            assert(this->tablePool.find(dtName) != this->tablePool.end());
            assert(this->tablePool[dtName]->colInfoMapping.find(dcName) != this->tablePool[dtName]->colInfoMapping.end());
            auto dest = this->tablePool[dtName]->colInfoMapping[dcName];
            for (unsigned int j=0;j<dest->referedBy.size();j++){
                if ((dest->referedBy[j].first == tableName) && (dest->referedBy[j].second == ti->colInfos[i]->columnName)){
                    dest->referedBy.erase(dest->referedBy.begin()+j);
                    break;
                }
            }
        }
    }

    this->tablePool[tableName] = nullptr;
    this->tablePool.erase(this->tablePool.find(tableName));
    unlink(string(tableName+".tbinfo").c_str());
    unlink(string(tableName+".tbdata").c_str());

    cout << "[Info] Dropped table " << tableName << ".\n";
}

void DatabaseManager::descTable(string tableName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "[Error] Table " << tableName << " doesn't exist!\n";
        return;
    }
    this->tablePool[tableName]->showTableInfo();
}

void DatabaseManager::openDataFile(shared_ptr<TableInfo> p){
    p->openDataFile();
}

bool DatabaseManager::addIndex(string tableName, string colName, int mode){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "[Error] Table " << tableName << " not found!\n";
        return false;
    }
    shared_ptr<TableInfo> ti = this->tablePool[tableName];

    if (ti->colInfoMapping.find(colName) == ti->colInfoMapping.end()){
        cout << "[Error] Column " << colName << " not found!\n";
        return false;
    }
    shared_ptr<ColumnInfo> ci = ti->colInfoMapping[colName];
    
    if (ci->indexTree != nullptr){
        cout << "[Info] Index already exists!\n";
        return false;
    }

    shared_ptr<BPlusTree> bt = make_shared<BPlusTree>(tableName, colName, ci->columnType);
    
    data_ptr ptr = ti->dataFile->firstData();
    while (ptr != nullptr){
        data_ptr raw = ti->cvt->getRawData(colName);
        bt->insert(raw, ti->dataFile->getCurrentRID().toInt());
        ptr = ti->dataFile->nextData();
    }

    ci->indexTree = bt;
    ci->useIndex = mode;
    return true;
}

bool DatabaseManager::dropIndex(string tableName, string colName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "[Error] Table " << tableName << " not found!\n";
        return false;
    }
    shared_ptr<TableInfo> ti = this->tablePool[tableName];

    if (ti->colInfoMapping.find(tableName) == ti->colInfoMapping.end()){
        cout << "[Error] Column " << colName << " not found!\n";
        return false;
    }
    shared_ptr<ColumnInfo> ci = ti->colInfoMapping[colName];

    if (ci->indexTree == nullptr){
        cout << "[Error] Index doesn't exist!\n";
        return false;
    }

    if (ci->isPrimary || ci->hasForeign){
        cout << "[Error] Cannot remove index on primary or foreign key!\n";
        return false;
    }

    ci->indexTree->deleteIndex();
    ci->indexTree = nullptr;
    ci->useIndex = 0;
    return true;
}