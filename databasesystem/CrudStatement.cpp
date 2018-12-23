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
    vector<WhereClause> primaryConstraint;
    for (unsigned int i=0;i<this->vList.size();i++){
        indexValueMap.clear();
        primaryConstraint.clear();
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

            varTypes ctype = cif->columnType;
            if (!CrudHelper::convertible(ctype, v.type)){
                cout << "[Error] Inconvertible type on column " << cif->columnName << "\n";
                flag = false;
                break;
            }
            
            if (cif->isPrimary){
                assert(cif->useIndex > 0);
                assert(cif->indexTree != nullptr);
                
                /*
                if (cif->indexTree->getRIDs(v.data).size() > 0){
                    cout << "[Error] Duplicate key on primary column" << cif->columnName << ".\n";
                    flag = false;
                }
                */

                WhereClause cons = CrudHelper::genConstraint(tif->tableName, cif->columnName, v);
                primaryConstraint.push_back(cons);
            }

            indexValueMap[j] = CrudHelper::convert(ctype, v, flag);
            if (!flag) break;
            
            if (cif->hasForeign){
                auto t = db->tablePool[cif->foreignTableName];
                assert(t != nullptr);
                auto c = t->colInfoMapping[cif->foreignColumnName];
                assert(c != nullptr);
                assert(c->indexTree != nullptr);
                if (!c->indexTree->has(indexValueMap[j])){
                    cout << "[Error] Failed to solve foreign key constraint on " << cif->columnName << "\n";
                    flag = false;
                    break;
                }
            }
        }

        
        if (primaryConstraint.size() > 0){
            if (primaryConstraint.size() == 1){
                auto cif = tif->colInfoMapping[primaryConstraint[0].col.colName];
                if (cif->indexTree->getRIDs(primaryConstraint[0].expr.val.data).size() > 0){
                    cout << "[Error] Duplicate key on primary column" << primaryConstraint[0].col.colName << ".\n";
                    flag = false;
                }
            }
            /*
            //Warning: significantly slows down the program 
            //Uncomment this block when needed
            else if (CrudHelper::getRIDsFrom(tif, primaryConstraint).size() > 0){
                cout << "[Error] Duplicate key on primary columns.\n";
                flag = false;
            }
            */
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

    cout << "[Debug] Inserted " << this->vList.size() << " records.\n";
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

    cout << "[Debug] Deleted " << res.size() << " records.\n";
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
        assert(tif != nullptr);

        if (tif->colInfoMapping.find(wc.col.colName) == tif->colInfoMapping.end()){
            cout << "[Error] Column " << wc.col.colName << " doesn't exist.\n";
            return;
        }
        varTypes t1 = tif->colInfoMapping[wc.col.colName]->columnType;
        if (wc.expr.type == ExprType::COL_EXPR){
            
            auto tif2 = db->tablePool[wc.expr.col.tableName];
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
            if (!CrudHelper::convertible(t1, t2)){
                cout << "[Error] Column " << wc.col.colName << " cannot be converted from type " << DataOperands::typeName(t2) << "\n";
                return;
            }
        }
    }

    cout << "[Debug] Check4 end.\n";

    //single table selection
    if (this->tList.size() == 1){
        cout << "[Debug] Single select begin.\n";
        auto tif = db->tablePool[this->tList[0]];
        vector<RID> res = CrudHelper::getRIDsFrom(tif, this->wcs);
        cout << "[Info] Selected " << res.size() << " records.\n";

        if (this->sel.type == SelectorType::COL_SELECTOR){
            cout << "(";
            for (unsigned int i=0;i<this->sel.cols.size();i++){
                if (i > 0){
                    cout << ",";
                }
                //cout << this->sel.cols[i].tableName << "." << this->sel.cols[i].colName;
                cout << this->sel.cols[i].colName;
            }
            cout << ")\n";

            for (unsigned int j=0;j<res.size();j++){
                cout << "(";
                for (unsigned int i=0;i<this->sel.cols.size();i++){
                    if (i > 0){
                        cout << ",";
                    }
                    RID r = res[j];
                    data_ptr raw = tif->dataFile->getData(r);
                    tif->cvt->fromByteArray(raw);
                    data_ptr key = tif->cvt->getRawData(this->sel.cols[i].colName);
                    varTypes type = tif->colInfoMapping[this->sel.cols[i].colName]->columnType;
                    cout << DataOperands::toString(type, key);
                }
                cout << ")\n";
            }
        } else if (this->sel.type == SelectorType::WILD_SELECTOR){
            cout << "(";

            for (int j=0;j<tif->colNumbers;j++){
                if (j > 0){
                    cout << ",";
                }
                cout << tif->colInfos[j]->columnName;
            }
            cout << ")\n";

            for (unsigned int k=0;k<res.size();k++){
                cout << "(";
                RID r = res[k];
                data_ptr raw = tif->dataFile->getData(r);
                tif->cvt->fromByteArray(raw);
                for (int j=0;j<tif->colNumbers;j++){
                    if (j > 0){
                        cout << ",";
                    }
                    data_ptr key = tif->cvt->getRawData(j);
                    varTypes type = tif->colInfos[j]->columnType;
                    cout << DataOperands::toString(type, key);
                }
                cout << ")\n";
            }
        }

        /*
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
        */
        return;
    }

    //multi table selection
    vector<string> sortedTables;
    vector<WhereClause> sortedWhereConditions;
    CrudHelper::sortCrossTables(db, this->tList, this->wcs, sortedTables, sortedWhereConditions);
    if (sortedTables.size() < this->tList.size()){
        cout << "[Error] Some tables are not constrained, abort.\n";
        return;
    }

    map<string, int> tableIndex;
    for (unsigned int i=0;i<sortedTables.size();i++){
        tableIndex[sortedTables[i]] = i;
    }

    int currentSolvedLength = 0;
    vector<vector<RID>> combinationResult;

    for (unsigned int i=0;i<sortedWhereConditions.size();i++){
        int weight;
        if ((sortedWhereConditions[i].expr.type == ExprType::VALUE_EXPR) || (sortedWhereConditions[i].col.tableName == sortedWhereConditions[i].expr.col.tableName)){
            //value condition or inter-table condition
            weight = tableIndex[sortedWhereConditions[i].col.tableName];
            auto tif = db->tablePool[sortedWhereConditions[i].col.tableName];

            if (weight == currentSolvedLength){
                //includes new table
                vector<RID> t = CrudHelper::getRIDsFrom(tif, sortedWhereConditions[i]);

                if (currentSolvedLength == 0){
                    //first table
                    for (RID r : t){
                        vector<RID> tt;
                        tt.push_back(r);
                        combinationResult.push_back(tt);
                    }
                } else {
                    //not first table
                    vector<vector<RID>> newComb;
                    for (vector<RID> vr : combinationResult){
                        for (RID r : t){
                            vector<RID> __vr = vr;
                            __vr.push_back(r);
                            newComb.push_back(__vr);
                        }
                    }
                    combinationResult = newComb;
                }
                currentSolvedLength++;
            } else {
                //only update existing table
                vector<vector<RID>> newComb;
                for (vector<RID> vr : combinationResult){
                    data_ptr toCheck = tif->dataFile->getData(vr[weight]);
                    if (CrudHelper::checkCondition(tif->cvt, toCheck, sortedWhereConditions[i])){
                        combinationResult.push_back(vr);
                    }
                }
                combinationResult = newComb;
            }
        } else {
            //cross-table condition
            weight = tableIndex[sortedWhereConditions[i].expr.col.tableName];
            if (weight == currentSolvedLength){
                //two new tables
                assert(weight+1 == tableIndex[sortedWhereConditions[i].col.tableName]);
                auto tif = db->tablePool[sortedWhereConditions[i].col.tableName];
                auto tif_source = db->tablePool[sortedWhereConditions[i].expr.col.tableName];
                //int index = tableIndex[sortedWhereConditions[i].col.tableName];
                //int index_source = tableIndex[sortedWhereConditions[i].expr.col.tableName];

                if (currentSolvedLength == 0){
                    //first table
                    data_ptr dataIterator = tif_source->dataFile->firstData();
                    vector<vector<RID>> newComb;

                    while (dataIterator != nullptr){
                        tif_source->cvt->fromByteArray(dataIterator);
                        data_ptr key = tif_source->cvt->getRawData(sortedWhereConditions[i].expr.col.colName);
                        varTypes type = tif_source->colInfoMapping[sortedWhereConditions[i].expr.col.colName]->columnType;
                        Value v(type, key);

                        WhereClause wc = CrudHelper::genConstraint(sortedWhereConditions[i].col.tableName, sortedWhereConditions[i].col.colName, v, sortedWhereConditions[i].op);
                        vector<RID> t = CrudHelper::getRIDsFrom(tif, wc);

                        for (RID r : t){
                            vector<RID> vr;
                            vr.push_back(tif_source->dataFile->getCurrentRID());
                            vr.push_back(r);
                            newComb.push_back(vr);
                        }
                        dataIterator = tif_source->dataFile->nextData();
                    }

                    combinationResult = newComb;
                } else {
                    //not first table
                    data_ptr dataIterator = tif_source->dataFile->firstData();
                    vector<vector<RID>> newComb;

                    while (dataIterator != nullptr){
                        tif_source->cvt->fromByteArray(dataIterator);
                        data_ptr key = tif_source->cvt->getRawData(sortedWhereConditions[i].expr.col.colName);
                        varTypes type = tif_source->colInfoMapping[sortedWhereConditions[i].expr.col.colName]->columnType;
                        Value v(type, key);

                        WhereClause wc = CrudHelper::genConstraint(sortedWhereConditions[i].col.tableName, sortedWhereConditions[i].col.colName, v, sortedWhereConditions[i].op);
                        vector<RID> t = CrudHelper::getRIDsFrom(tif, wc);

                        for (vector<RID> vr : combinationResult){
                            for (RID r : t){
                                vector<RID> __vr = vr;
                                __vr.push_back(tif_source->dataFile->getCurrentRID());
                                __vr.push_back(r);
                                newComb.push_back(__vr);
                            }
                        }
                        
                        dataIterator = tif_source->dataFile->nextData();
                    }

                    combinationResult = newComb;
                }

                currentSolvedLength += 2;
            } else {
                //includes at least existing table
                weight = tableIndex[sortedWhereConditions[i].col.tableName];
                auto tif = db->tablePool[sortedWhereConditions[i].col.tableName];
                auto tif_source = db->tablePool[sortedWhereConditions[i].expr.col.tableName];
                int index = tableIndex[sortedWhereConditions[i].expr.col.tableName];
                
                if (weight >= currentSolvedLength){
                    //new table
                    assert(weight == currentSolvedLength);

                    vector<vector<RID>> newComb;
                    for (vector<RID> vr : combinationResult){
                        data_ptr raw = tif_source->dataFile->getData(vr[index]);
                        tif_source->cvt->fromByteArray(raw);
                        data_ptr key = tif_source->cvt->getRawData(sortedWhereConditions[i].expr.col.colName);
                        varTypes type = tif_source->colInfoMapping[sortedWhereConditions[i].expr.col.colName]->columnType;
                        Value v(type, key);

                        WhereClause wc = CrudHelper::genConstraint(sortedWhereConditions[i].col.tableName, sortedWhereConditions[i].col.colName, v, sortedWhereConditions[i].op);
                        vector<RID> t = CrudHelper::getRIDsFrom(tif, wc);

                        for (RID r : t){
                            vector<RID> __vr = vr;
                            __vr.push_back(r);
                            newComb.push_back(__vr);
                        }
                    }
                    combinationResult = newComb;

                    currentSolvedLength++;
                } else {
                    //existing table
                    vector<vector<RID>> newComb;
                    for (vector<RID> vr : combinationResult){
                        data_ptr raw = tif_source->dataFile->getData(vr[index]);
                        tif_source->cvt->fromByteArray(raw);
                        data_ptr key = tif_source->cvt->getRawData(sortedWhereConditions[i].expr.col.colName);
                        varTypes type = tif_source->colInfoMapping[sortedWhereConditions[i].expr.col.colName]->columnType;
                        Value v(type, key);
                        
                        WhereClause wc = CrudHelper::genConstraint(sortedWhereConditions[i].col.tableName, sortedWhereConditions[i].col.colName, v, sortedWhereConditions[i].op);

                        data_ptr toCheck = tif->dataFile->getData(vr[weight]);
                        if (CrudHelper::checkCondition(tif->cvt, toCheck, wc)){
                            combinationResult.push_back(vr);
                        }
                    }
                    combinationResult = newComb;
                }
            }
        }
    }

    cout << "[Info] Selected " << combinationResult.size() << " results.\n";

    /*
    cout << "[Debug] RIDs:\n";
    for (vector<RID> rv : combinationResult){
        cout << "(";
        for (RID r : rv){
            cout << r.pagenum << "." << r.slotnum << ",";
        }
        cout << ")\n";
    }
    */

    if (this->sel.type == SelectorType::COL_SELECTOR){
        cout << "(";
        for (unsigned int i=0;i<this->sel.cols.size();i++){
            if (i > 0){
                cout << ",";
            }
            cout << this->sel.cols[i].tableName << "." << this->sel.cols[i].colName;
        }
        cout << ")\n";

        for (unsigned int j=0;j<combinationResult.size();j++){
            cout << "(";
            for (unsigned int i=0;i<this->sel.cols.size();i++){
                if (i > 0){
                    cout << ",";
                }
                auto tif = db->tablePool[this->sel.cols[i].tableName];
                RID r = combinationResult[j][tableIndex[this->sel.cols[i].tableName]];
                data_ptr raw = tif->dataFile->getData(r);
                tif->cvt->fromByteArray(raw);
                data_ptr key = tif->cvt->getRawData(this->sel.cols[i].colName);
                varTypes type = tif->colInfoMapping[this->sel.cols[i].colName]->columnType;
                cout << DataOperands::toString(type, key);
            }
            cout << ")\n";
        }
    } else if (this->sel.type == SelectorType::WILD_SELECTOR){
        cout << "(";
        for (unsigned int i=0;i<this->tList.size();i++){
            auto tif = db->tablePool[this->tList[i]];
            for (int j=0;j<tif->colNumbers;j++){
                if ((i > 0) || (j > 0)){
                    cout << ",";
                }
                cout << tif->tableName << "." << tif->colInfos[j]->columnName;
            }
        }
        cout << ")\n";

        for (unsigned int k=0;k<combinationResult.size();k++){
            cout << "(";
            for (unsigned int i=0;i<this->tList.size();i++){
                auto tif = db->tablePool[this->tList[i]];
                RID r = combinationResult[k][tableIndex[this->sel.cols[i].tableName]];
                data_ptr raw = tif->dataFile->getData(r);
                tif->cvt->fromByteArray(raw);
                for (int j=0;j<tif->colNumbers;j++){
                    if ((i > 0) || (j > 0)){
                        cout << ",";
                    }
                    data_ptr key = tif->cvt->getRawData(j);
                    varTypes type = tif->colInfos[j]->columnType;
                    cout << DataOperands::toString(type, key);
                }
            }
            cout << ")\n";
        }
    }
}