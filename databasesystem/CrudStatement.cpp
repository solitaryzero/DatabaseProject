#include "CrudStatement.h"

InsertStatement::InsertStatement(string *tableName, vector<ValueList> *valueLists){
    this->tbName = *tableName;
    this->vList = *valueLists;
    delete tableName;
    delete valueLists;
}

InsertStatement::~InsertStatement(){

}

void InsertStatement::run(DatabaseManager *db){
    if (db == nullptr){
        cout << "[Error] Not using any database!\n";
        return;
    }

    if (db->tablePool.find(this->tbName) == db->tablePool.end()){
        cout << "[Error] Table " << this->tbName << " doesn't exist!\n";
        return;
    }

    shared_ptr<TableInfo> tif = db->tablePool[this->tbName];
    if (tif->cvt == nullptr){
        cout << "[Error] Incomplete table\n";
        return;
    }
    
    map<int, data_ptr> indexValueMap;
    for (unsigned int i=0;i<this->vList.size();i++){
        indexValueMap.clear();
        bool flag = true;
        
        for (unsigned int j=0;j<this->vList[i].values.size();j++){
            if (this->vList[i].values.size() != tif->colNumbers){
                cout << "[Error] Wrong column number for valuelist No." << i << "\n";
                flag = false;
                break;
            }

            Value v = this->vList[i].values[j];
            auto cif = tif->colInfos[j];
            if (v.type == varTypes::UNKNOWN_TYPE){
                assert(v.data == nullptr);
                if (!cif->allowNull){
                    cout << "[Error] Trying to insert null into a not nullable column for valueList No." << i << "\n";
                    flag = false;
                    break;
                }
                indexValueMap[j] = nullptr;
            }

            if (cif->isPrimary){
                assert(cif->useIndex > 0);
                assert(cif->indexTree != nullptr);
                if (cif->indexTree->has(v.data)){
                    cout << "[Error] Duplicate key " << v.raw << " on primary column " << cif->columnName << "\n";
                    flag = false;
                    break;
                }
            }

            varTypes ctype = cif->columnType;
            if (!CrudHelper::convertible(ctype, v.type)){
                cout << "[Error] Inconvertible type on column " << cif->columnName << "\n";
                flag = false;
                break;
            }
            
            indexValueMap[j] = CrudHelper::convert(ctype, v, flag);
            if (!flag) break;

            if (cif->hasForeign){
                auto t = db->tablePool[cif->foreignTableName];
                auto c = t->colInfoMapping[cif->foreignColumnName];
                if (!c->indexTree->has(indexValueMap[j])){
                    cout << "[Error] Failed to solve foreign key constraint on " << cif->columnName << "\n";
                    flag = false;
                    break;
                }
            }
        }
        
        if (flag){
            tif->cvt->fromIndexValueMap(indexValueMap);
            RID res = tif->dataFile->insertData(tif->cvt->toByteArray());

            for (int j=0;j<tif->colNumbers;j++){
                if (tif->colInfos[j]->useIndex && (indexValueMap[j] != nullptr)){
                    if (tif->colInfos[j]->indexTree == nullptr){
                        cout << "[Error] Broken index on column " << tif->colInfos[j]->columnName << "\n";
                    } else {
                        tif->colInfos[j]->indexTree->insert(indexValueMap[j], res.toInt());
                    }
                }
            }
        } else {
            cout << "[Warning] Error detected in valueList No." << i << ", abort to insert it.\n";
        }
    }
}

DeleteStatement::DeleteStatement(string *tableName, vector<WhereClause> *whereClauses){
    this->tbName = *tableName;
    this->wcs = *whereClauses;
    delete tableName;
    delete whereClauses;
}

DeleteStatement::~DeleteStatement(){

}

void DeleteStatement::run(DatabaseManager *db){
    if (db->tablePool.find(this->tbName) == db->tablePool.end()){
        cout << "[Error] Table " << this->tbName << " doesn't exist!\n";
        return;
    }
    auto tif = db->tablePool[this->tbName];
    
    for (WhereClause wc : this->wcs){
        if (tif->colInfoMapping.find(wc.col.colName) == tif->colInfoMapping.end()){
            cout << "[Error] Column " << wc.col.colName << " doesn't exist.\n";
            return;
        }
        varTypes t1 = tif->colInfoMapping[wc.col.colName]->columnType;
        if (wc.expr.type == ExprType::COL_EXPR){
            if (tif->colInfoMapping.find(wc.expr.col.colName) == tif->colInfoMapping.end()){
                cout << "[Error] Column " << wc.expr.col.colName << " doesn't exist.\n";
                return;
            }
            varTypes t2 = tif->colInfoMapping[wc.expr.col.colName]->columnType;
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " and " << wc.expr.col.colName << " have different types.\n";
                return;
            }
        } else {
            varTypes t2 = wc.expr.val.type;
            if (!CrudHelper::convertible(t1, t2)){
                cout << "[Error] Column " << wc.col.colName << " cannot be converted from type " << DataOperands::typeName(t2) << "\n";
                return;
            }
        }
    }

    vector<RID> res = CrudHelper::getRIDsFrom(tif, this->wcs);
    CrudHelper::solveForeignKey_delete(db, tif, res);
}

UpdateStatement::UpdateStatement(string *tableName, vector<SetClause> *setClauses, vector<WhereClause> *whereClauses){
    this->tbName = *tableName;
    this->setClauses = *setClauses;
    this->wcs = *whereClauses;
    delete tableName;
    delete setClauses;
    delete whereClauses;
}

UpdateStatement::~UpdateStatement(){
    
}

void UpdateStatement::run(DatabaseManager *db){
    if (db->tablePool.find(this->tbName) == db->tablePool.end()){
        cout << "[Error] Table " << this->tbName << " doesn't exist!\n";
        return;
    }
    auto tif = db->tablePool[this->tbName];

    for (WhereClause wc : this->wcs){
        if (tif->colInfoMapping.find(wc.col.colName) == tif->colInfoMapping.end()){
            cout << "[Error] Column " << wc.col.colName << " doesn't exist.\n";
            return;
        }
        varTypes t1 = tif->colInfoMapping[wc.col.colName]->columnType;
        if (wc.expr.type == ExprType::COL_EXPR){
            if (tif->colInfoMapping.find(wc.expr.col.colName) == tif->colInfoMapping.end()){
                cout << "[Error] Column " << wc.expr.col.colName << " doesn't exist.\n";
                return;
            }
            varTypes t2 = tif->colInfoMapping[wc.expr.col.colName]->columnType;
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " and " << wc.expr.col.colName << " have different types.\n";
                return;
            }
        } else {
            varTypes t2 = wc.expr.val.type;
            if (!CrudHelper::convertible(t1, t2)){
                cout << "[Error] Column " << wc.col.colName << " cannot be converted from type " << DataOperands::typeName(t2) << "\n";
                return;
            }
        }
    }

    for (SetClause sc : this->setClauses){
        if (tif->colInfoMapping.find(sc.colName) == tif->colInfoMapping.end()){
            cout << "[Error] Column " << sc.colName << " doesn't exist.\n";
            return;
        }
        auto cif = tif->colInfoMapping[sc.colName];

        if (cif->isPrimary){
            cout << "[Error] Cannot update primary key on column " << sc.colName << ".\n";
            return;
        }

        if (!CrudHelper::convertible(cif->columnType, sc.v.type)){
            cout << "[Error] Inconvertible type on column " << cif->columnName << "\n";
            return;
        }

        if (cif->hasForeign){
            auto t = db->tablePool[cif->foreignTableName];
            auto c = t->colInfoMapping[cif->foreignColumnName];
            bool flag = true;
            if (!c->indexTree->has(CrudHelper::convert(cif->columnType, sc.v, flag))){
                cout << "[Error] Failed to solve foreign key constraint on " << cif->columnName << "\n";
                return;
            }
        }
    }

    vector<RID> res = CrudHelper::getRIDsFrom(tif, this->wcs);
    vector<data_ptr> oldKeys;
    vector<data_ptr> newKeys;
    for (RID r : res){
        oldKeys.clear();
        newKeys.clear();
        data_ptr oldData = tif->dataFile->getData(r);
        tif->cvt->fromByteArray(oldData);

        for (SetClause sc : this->setClauses){
            auto cif = tif->colInfoMapping[sc.colName];
            if (cif->useIndex > 0){
                data_ptr oldKey = tif->cvt->getRawData(sc.colName);
                bool flag = true;
                data_ptr newKey = CrudHelper::convert(cif->columnType, sc.v, flag);

                tif->cvt->setRawData(sc.colName, newKey);
                if (!flag){
                    cout << "[Error] Update failed on column " << sc.colName << ".\n";
                    return;
                }
            } else {
                bool flag = true;
                tif->cvt->setRawData(sc.colName, CrudHelper::convert(cif->columnType, sc.v, flag));
                if (!flag){
                    cout << "[Error] Update failed on column " << sc.colName << ".\n";
                    return;
                }
            }
        }
        data_ptr newData = tif->cvt->toByteArray();
        RID newRID = tif->dataFile->updateData(r, newData);
        
        if (newRID == r) continue;

        for (auto cif : tif->colInfos){
            if (cif->useIndex == 0) continue;
            tif->cvt->fromByteArray(oldData);
            data_ptr oldKey = tif->cvt->getRawData(cif->columnName);
            tif->cvt->fromByteArray(newData);
            data_ptr newKey = tif->cvt->getRawData(cif->columnName);
            cif->indexTree->remove(oldKey, r.toInt());
            cif->indexTree->insert(newKey, newRID.toInt());
        }

    }
}

SelectStatement::SelectStatement(Selector *sel, vector<string> *tableList, vector<WhereClause> *whereClauses){
    this->sel = *sel;
    this->tList = *tableList;
    this->wcs = *whereClauses;
    delete sel;
    delete tableList;
    delete whereClauses;
}

SelectStatement::~SelectStatement(){
    
}

void SelectStatement::run(DatabaseManager *db){
    //check table name correctness
    for (unsigned int i=0;i<this->tList.size();i++){
        if (db->tablePool.find(this->tList[i]) == db->tablePool.end()){
            cout << "[Error] Table " << this->tList[i] << " doesn't exist.\n";
            return;
        }

        for (unsigned int j=i+1;j<this->tList.size();j++){
            if (this->tList[i] == this->tList[j]){
                cout << "[Error] Duplicate table name " << this->tList[i] << ".\n";
                return;
            }
        }
    }

    //check selector column correctness
    if (this->sel.type == SelectorType::COL_SELECTOR){
        for (Column &c : this->sel.cols){
            bool res = CrudHelper::getTableName(db, c, this->tList);
            if (!res) return;
        }
    }

    //check whereclause column correctness
    for (WhereClause &wc : this->wcs){
        bool res = CrudHelper::getTableName(db, wc.col, this->tList);
        if (!res) return;

        if (wc.expr.type == ExprType::COL_EXPR){
            res = CrudHelper::getTableName(db, wc.expr.col, this->tList);
            if (!res) return;
        }
    }

    //check whereclause correctness
    for (WhereClause &wc : this->wcs){
        auto tif = db->tablePool[wc.col.tableName];

        cout << wc.col.tableName << "\n";
        assert(tif != nullptr);

        if (tif->colInfoMapping.find(wc.col.colName) == tif->colInfoMapping.end()){
            cout << "[Error] Column " << wc.col.colName << " doesn't exist.\n";
            return;
        }
        varTypes t1 = tif->colInfoMapping[wc.col.colName]->columnType;
        if (wc.expr.type == ExprType::COL_EXPR){
            auto tif2 = db->tablePool[wc.expr.col.colName];
            if (tif2->colInfoMapping.find(wc.expr.col.colName) == tif2->colInfoMapping.end()){
                cout << "[Error] Column " << wc.expr.col.colName << " doesn't exist.\n";
                return;
            }
            varTypes t2 = tif2->colInfoMapping[wc.expr.col.colName]->columnType;
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " and " << wc.expr.col.colName << " have different types.\n";
                return;
            }
        } else {
            varTypes t2 = wc.expr.val.type;
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " doesn't have type " << DataOperands::typeName(t2) << "\n";
                return;
            }
        }
    }

    //TODO: implement cross-table query
    auto tif = db->tablePool[this->tList[0]];
    vector<RID> res = CrudHelper::getRIDsFrom(tif, this->wcs);

    if (this->sel.type == SelectorType::WILD_SELECTOR){
        for (int i=0;i<tif->colNumbers;i++){
            printf("%-12s", tif->colInfos[i]->columnName.c_str());
        }
        printf("\n");

        for (RID r : res){
            data_ptr d = tif->dataFile->getData(r);
            tif->cvt->fromByteArray(d);
            for (int i=0;i<tif->colNumbers;i++){
                printf("%-12s", DataOperands::toString(tif->colInfos[i]->columnType, tif->cvt->getRawData(i)).c_str());
            }
            printf("\n");
        }
    } else if (this->sel.type == SelectorType::COL_SELECTOR){
        for (Column &c : this->sel.cols){
            printf("%-12s", c.colName.c_str());
        }
        printf("\n");

        for (RID r : res){
            data_ptr d = tif->dataFile->getData(r);
            assert(d != nullptr);
            tif->cvt->fromByteArray(d);
            for (Column &c : this->sel.cols){
                printf("%-12s", DataOperands::toString(tif->colInfoMapping[c.colName]->columnType, tif->cvt->getRawData(c.colName)).c_str());
            }
            printf("\n");
        }
    }
}