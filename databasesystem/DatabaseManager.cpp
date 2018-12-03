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
        cout << "open directory failed\n";
        return;
    }

    cout << "Databases\n";
    cout << "================\n";
    struct dirent *filename;
    while((filename = readdir(dir)) != nullptr){
        string fname(filename->d_name);
        if ((filename->d_type == DT_DIR) && (fname != ".") && (fname != "..")){
            cout << fname << "\n";
        }
    } 
    cout << "================\n";
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
}

void DatabaseManager::createDatabase(string dbName){
    string fullPath = this->pathBase+"/"+dbName;
    if (access(fullPath.c_str(), F_OK) == 0){
        cout << "database" << dbName << "already exists!\n";
        return;
    }

    mkdir(fullPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void DatabaseManager::dropDatabase(string dbName){
    string fullPath = this->pathBase+"/"+dbName;
    if (access(fullPath.c_str(), F_OK) != 0){
        cout << "database" << dbName << "doesn't exist!\n";
        return;
    }

    if (this->databaseName == dbName){
        this->databaseName == "";
    }
    rmdir(fullPath.c_str());
}

void DatabaseManager::showTables(){
    if (this->databaseName == ""){
        cout << "Not using any database!\n";
        return;
    }

    cout << "Tables in " << this->databaseName << ":\n";
    cout << "================\n";

    if (this->tablePool.size() == 0){
        cout << "No tables found.\n";
        return;
    }

    auto it = this->tablePool.begin();
    while (it != this->tablePool.end()){
        cout << it->first << "\n";
    }

    cout << "================\n";
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

shared_ptr<TableInfo> DatabaseManager::createTable(string tableName, vector<pair<string, varTypes>> cols, vector<int> sizes){
    if (this->tablePool.find(tableName) != this->tablePool.end()){
        cout << "Table " << tableName << " already exists!\n";
        return nullptr;
    }
    shared_ptr<TableInfo> tif = createTable(tableName);
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

void DatabaseManager::descTable(string tableName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "Table " << tableName << " doesn't exist!\n";
        return;
    }
    this->tablePool[tableName]->showTableInfo();
}

void DatabaseManager::openDataFile(shared_ptr<TableInfo> p){
    p->openDataFile();
}

bool DatabaseManager::addIndex(string tableName, string colName, int mode){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "Table " << tableName << " not found!\n";
        return false;
    }
    shared_ptr<TableInfo> ti = this->tablePool[tableName];

    if (ti->colInfoMapping.find(tableName) == ti->colInfoMapping.end()){
        cout << "Column " << colName << " not found!\n";
        return false;
    }
    shared_ptr<ColumnInfo> ci = ti->colInfoMapping[colName];
    
    if (ci->indexTree != nullptr){
        cout << "Index already exists!\n";
        return false;
    }

    shared_ptr<BPlusTree> bt = make_shared<BPlusTree>(tableName, colName, ci->columnType);
    ci->indexTree = bt;
    return true;
}

bool DatabaseManager::dropIndex(string tableName, string colName){
    if (this->tablePool.find(tableName) == this->tablePool.end()){
        cout << "Table " << tableName << " not found!\n";
        return false;
    }
    shared_ptr<TableInfo> ti = this->tablePool[tableName];

    if (ti->colInfoMapping.find(tableName) == ti->colInfoMapping.end()){
        cout << "Column " << colName << " not found!\n";
        return false;
    }
    shared_ptr<ColumnInfo> ci = ti->colInfoMapping[colName];

    if (ci->indexTree == nullptr){
        cout << "Index doesn't exist!\n";
        return false;
    }

    ci->indexTree->deleteIndex();
    ci->indexTree = nullptr;
    return true;
}