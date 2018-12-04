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
        } else if ((this->fieldList[i].mode == FIELD_PRIMARY) || (this->fieldList[i].mode == FIELD_FOREIGN)){
            // pass
        } else {
            cout << "Unknown field\n";
        }
    }
    db->createTable(this->tbName, cols, sizes);
    
    for (int i=0;i<db->tablePool[this->tbName]->colNumbers;i++){
        shared_ptr<ColumnInfo> ci = db->tablePool[this->tbName]->colInfos[i];
        if (ci->columnType == varTypes::INT_TYPE){
            ci->showLength = ci->size;
            ci->size = DataOperands::getTypeSize(varTypes::INT_TYPE);
        }
    }

    for (unsigned int i=0;i<this->fieldList.size();i++){
        if (this->fieldList[i].mode == FIELD_NOTNULL){
            db->tablePool[this->tbName]->colInfoMapping[this->fieldList[i].name]->allowNull = false;
        } else if (this->fieldList[i].mode == FIELD_PRIMARY){
            for (string ss : this->fieldList[i].colNames){
                db->tablePool[this->tbName]->colInfoMapping[ss]->setPrimary(true);
            }
        } else if (this->fieldList[i].mode == FIELD_FOREIGN){
            if (db->tablePool.find(this->fieldList[i].destTabName) == db->tablePool.end()){
                cout << "destination (" << this->fieldList[i].destTabName << "." << this->fieldList[i].destColName << ") doesn't exist!\n";
                continue;
            }
            shared_ptr<TableInfo> ti = db->tablePool[this->fieldList[i].destTabName];

            if (ti->colInfoMapping.find(this->fieldList[i].destColName) == ti->colInfoMapping.end()){
                cout << "destination (" << this->fieldList[i].destTabName << "." << this->fieldList[i].destColName << ") doesn't exist!\n";
                continue;
            }
            shared_ptr<ColumnInfo> ci = ti->colInfoMapping[this->fieldList[i].destColName];
            if (!ci->isPrimary){
                cout << "destination (" << this->fieldList[i].destTabName << "." << this->fieldList[i].destColName << ") is not primary!\n";
                continue;
            }

            shared_ptr<ColumnInfo> sci = db->tablePool[this->tbName]->colInfoMapping[this->fieldList[i].srcColName];
            if (sci->hasForeign){
                cout << "redefinition of foreign key on column " << this->fieldList[i].srcColName << "\n";
                continue;
            }

            sci->hasForeign = true;
            sci->foreignTableName = this->fieldList[i].destTabName;
            sci->foreignColumnName = this->fieldList[i].destColName;
            ci->referedBy.push_back({this->tbName, this->fieldList[i].srcColName});
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