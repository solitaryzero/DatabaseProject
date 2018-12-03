#include "Statement.h"

void ShowDatabaseStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->showDatabases();
}

CreateDatabaseStatement::CreateDatabaseStatement(string *name){
    this->dbName = *name;
    delete name;
}

void CreateDatabaseStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->createDatabase(this->dbName);
}

DropDatabaseStatement::DropDatabaseStatement(string *name){
    this->dbName = *name;
    delete name;
}

void DropDatabaseStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->dropDatabase(this->dbName);
}

UseDatabaseStatement::UseDatabaseStatement(string *name){
    this->dbName = *name;
    delete name;
}

void UseDatabaseStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->switchDatabase(this->dbName);
}

void ShowTableStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->showTables();
}

CreateTableStatement::CreateTableStatement(string *name, vector<Field> *fieldList){
    this->tbName = *name;
    this->fieldList = *fieldList;
    delete name;
    delete fieldList;
}

void CreateTableStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    vector<pair<string, varTypes>> cols;
    vector<int> sizes;
    for (unsigned int i=0;i<this->fieldList.size();i++){
        if ((this->fieldList[i].mode == FIELD_COMMON) || (this->fieldList[i].mode == FIELD_NOTNULL)){
            cols.push_back({fieldList[i].name, fieldList[i].t.t});
            sizes.push_back({fieldList[i].t.size});
        } else if (this->fieldList[i].mode == FIELD_PRIMARY){
            // pass
        } else {
            cout << "Unknown field\n";
        }
    }
    db->createTable(this->tbName, cols, sizes);
    
    for (unsigned int i=0;i<this->fieldList.size();i++){
        if (this->fieldList[i].mode == FIELD_NOTNULL){
            db->tablePool[this->tbName]->colInfoMapping[this->fieldList[i].name]->allowNull = false;
        } else if (this->fieldList[i].mode == FIELD_PRIMARY){
            db->tablePool[this->tbName]->colInfoMapping[this->fieldList[i].name]->setPrimary(true);
        }
    }
}

DropTableStatement::DropTableStatement(string *name){
    this->tbName = *name;
    delete name;
}

void DropTableStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->dropTable(this->tbName);
}

DescTableStatement::DescTableStatement(string *name){
    this->tbName = *name;
    delete name;
}

void DescTableStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->descTable(this->tbName);
}

CreateIndexStatement::CreateIndexStatement(string *tbName, string *colName){
    this->tbName = *tbName;
    this->colName = *colName;
    delete tbName;
    delete colName;
}

void CreateIndexStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->addIndex(this->tbName, this->colName);
}

DropIndexStatement::DropIndexStatement(string *tbName, string *colName){
    this->tbName = *tbName;
    this->colName = *colName;
    delete tbName;
    delete colName;
}

void DropIndexStatement::run(DatabaseManager *db){
    assert(db != nullptr);
    db->dropIndex(this->tbName, this->colName);
}