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
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " doesn't have type " << DataOperands::typeName(t2) << "\n";
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
            if (t1 != t2){
                cout << "[Error] Column " << wc.col.colName << " doesn't have type " << DataOperands::typeName(t2) << "\n";
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
        data_ptr old = tif->dataFile->getData(r);
        tif->cvt->fromByteArray(old);

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

                oldKeys.push_back(oldKey);
                newKeys.push_back(newKey);
            } else {
                bool flag = true;
                tif->cvt->setRawData(sc.colName, CrudHelper::convert(cif->columnType, sc.v, flag));
                if (!flag){
                    cout << "[Error] Update failed on column " << sc.colName << ".\n";
                    return;
                }

                oldKeys.push_back(nullptr);
                newKeys.push_back(nullptr);
            }
        }
        RID newRID = tif->dataFile->updateData(r, tif->cvt->toByteArray());
        for (unsigned int i=0;i<oldKeys.size();i++){
            auto cif = tif->colInfoMapping[this->setClauses[i].colName];
            if (oldKeys[i] != nullptr){
                cif->indexTree->remove(oldKeys[i], r.toInt());
            }
            if (newKeys[i] != nullptr){
                cif->indexTree->insert(newKeys[i], newRID.toInt());
            }
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
    
}

bool CrudHelper::convertible(varTypes to, varTypes from){
    switch(to){
        case varTypes::FLOAT_TYPE:
            return ((from == varTypes::INT_TYPE) || (from == varTypes::FLOAT_TYPE));
            break;
        case varTypes::VARCHAR_TYPE:
            return ((from == varTypes::CHAR_TYPE) || (from == varTypes::VARCHAR_TYPE));
            break;
        case varTypes::DATE_TYPE:
            return ((from == varTypes::DATE_TYPE) || (from == varTypes::CHAR_TYPE));
            break;
        case varTypes::DECIMAL_TYPE:
            return ((from == varTypes::INT_TYPE) || (from == varTypes::FLOAT_TYPE) || (from == varTypes::DECIMAL_TYPE));
            break;
        default:
            return (to == from);
            break;
    }
    return false;
}

data_ptr CrudHelper::convert(varTypes dest, Value &v, bool &success){
    switch (dest)
    {
        case varTypes::INT_TYPE:
            assert(v.type == varTypes::INT_TYPE);
            return v.data;
            break;

        case varTypes::FLOAT_TYPE:
            assert((v.type == varTypes::INT_TYPE) || (v.type == varTypes::FLOAT_TYPE));
            if (v.type == varTypes::FLOAT_TYPE){
                return v.data;
            } else {
                return Value::intToFloat(v);
            }
            break;

        case varTypes::CHAR_TYPE:
            assert(v.type == varTypes::CHAR_TYPE);
            return v.data;
            break;
        
        case varTypes::VARCHAR_TYPE:
            assert(v.type == varTypes::CHAR_TYPE);
            return v.data;
            break;

        case varTypes::DATE_TYPE:
        {
            assert(v.type == varTypes::CHAR_TYPE);
            data_ptr p = Value::stringToDate(v);
            if (p == nullptr){
                cout << "[Error] Bad date format\n";
                success = false;
                return nullptr;
            } else {
                return p;
            }
            break;
        }

        case varTypes::DECIMAL_TYPE:
            assert((v.type == varTypes::INT_TYPE) || (v.type == varTypes::FLOAT_TYPE));
            if (v.type == varTypes::INT_TYPE){
                return Value::intToDecimal(v);
            } else {
                return Value::floatToDecimal(v);
            }
            
            break;
    
        default:
            cout << "[Error] Unknown type\n";
            success = false;
            return nullptr;
            break;
    }
}

bool CrudHelper::checkCondition(shared_ptr<RecordConverter> cvt, data_ptr data, const WhereClause &wc){
    if (wc.col.hasTableName && (wc.col.tableName != cvt->tinfo->tableName)){
        return false;
    }

    cvt->fromByteArray(data);
    data_ptr a,b;
    varTypes type = cvt->tinfo->colInfoMapping[wc.col.colName]->columnType;
    a = cvt->getRawData(wc.col.colName);
    if ((wc.op != WhereOperands::WHERE_OP_ISNULL) && (wc.op != WhereOperands::WHERE_OP_NOTNULL)){
        if (wc.expr.type == ExprType::COL_EXPR){
            b = cvt->getRawData(wc.expr.col.colName);
        } else {
            b = wc.expr.val.data;
        }
    }

    switch(wc.op){
        case WhereOperands::WHERE_OP_EQ:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) == 0);
            break;
        case WhereOperands::WHERE_OP_NE:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) != 0);
            break;
        case WhereOperands::WHERE_OP_GT:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) > 0);
            break;
        case WhereOperands::WHERE_OP_LT:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) < 0);
            break;
        case WhereOperands::WHERE_OP_GE:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) >= 0);
            break;
        case WhereOperands::WHERE_OP_LE:
            if (a == nullptr) return false;
            if (b == nullptr) return false;
            return (DataOperands::compare(type, a, b) <= 0);
            break;
        case WhereOperands::WHERE_OP_ISNULL:
            if (a == nullptr) return true;
            return false;
            break;
        case WhereOperands::WHERE_OP_NOTNULL:
            if (a == nullptr) return false;
            return true;
            break;
        default:
            cout << "[Error] Undefined operand\n";
            return false;
            break;
    }
}

int CrudHelper::getCount(shared_ptr<TableInfo> tif, const WhereClause &wc){
    if (wc.expr.type == ExprType::COL_EXPR){
        return CrudHelper::failed;
    }
    auto cif = tif->colInfoMapping[wc.col.colName];
    if (!cif->useIndex){
        return CrudHelper::failed;
    }

    auto ind = cif->indexTree;
    assert(ind != nullptr);
    switch(wc.op){
        case WhereOperands::WHERE_OP_EQ:
            return ind->getRIDs(wc.expr.val.data).size();
            break;
        case WhereOperands::WHERE_OP_NE:
            return ind->totalCount()-ind->getRIDs(wc.expr.val.data).size();
            break;
        case WhereOperands::WHERE_OP_GT:
            return ind->greaterCount(wc.expr.val.data);
            break;
        case WhereOperands::WHERE_OP_LT:
            return ind->lesserCount(wc.expr.val.data);
            break;
        case WhereOperands::WHERE_OP_GE:
            return ind->totalCount()-ind->lesserCount(wc.expr.val.data);
            break;
        case WhereOperands::WHERE_OP_LE:
            return ind->totalCount()-ind->greaterCount(wc.expr.val.data);
            break;
        default:
            return CrudHelper::failed;
            break;
    }
    return CrudHelper::failed;
}

vector<RID> CrudHelper::getRIDsFrom(shared_ptr<TableInfo> tif, const vector<WhereClause> &wcs){
    vector<RID> res;
    if (wcs.size() == 0){
        auto it = tif->dataFile->firstData();
        while (it != nullptr){
            res.push_back(tif->dataFile->getCurrentRID());
            it = tif->dataFile->nextData();
        }
        return res;
    }

    int minCount = CrudHelper::failed;
    unsigned int pos = 0;
    for (unsigned int i=0;i<wcs.size();i++){
        int c = CrudHelper::getCount(tif, wcs[i]);
        if (c < minCount){
            minCount = c;
            pos = i;
        }
    }

    vector<RID> _res = CrudHelper::getRIDsFrom(tif, wcs[pos]);
    assert((minCount == CrudHelper::failed) || ((unsigned int)minCount == res.size()));
    for (RID r : _res){
        bool flag = true;
        for (unsigned int i=0;i<wcs.size();i++){
            if (i == pos) continue;
            data_ptr p = tif->dataFile->getData(r);
            if (!CrudHelper::checkCondition(tif->cvt, p, wcs[i])){
                flag = false;
                break;
            }
        }
        if (flag){
            res.push_back(r);
        }
    }

    return res;
}

vector<RID> CrudHelper::getRIDsFrom(shared_ptr<TableInfo> tif, const WhereClause &wc){
    auto cif = tif->colInfoMapping[wc.col.colName];
    vector<RID> res;
    if ((!cif->useIndex) || (wc.expr.type == ExprType::COL_EXPR)){
        data_ptr it = tif->dataFile->firstData();
        while (it != nullptr){
            if (CrudHelper::checkCondition(tif->cvt, it, wc)){
                res.push_back(tif->dataFile->getCurrentRID());
            }
            it = tif->dataFile->nextData();
        }
        return res;
    }

    auto ind = cif->indexTree;
    switch (wc.op){
        case WhereOperands::WHERE_OP_EQ:
            return ind->getRIDs(wc.expr.val.data);
            break;
        case WhereOperands::WHERE_OP_NE:
            {
                BPlusTreeIterator bit = ind->begin();
                while (bit.available() && (DataOperands::compare(cif->columnType, bit.getKey(), wc.expr.val.data) < 0)){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                bit = ind->upperBound(wc.expr.val.data);
                while (bit.available()){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                return res;
            }
            break;
        case WhereOperands::WHERE_OP_GT:
            {
                BPlusTreeIterator bit = ind->upperBound(wc.expr.val.data);

                while (bit.available()){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                return res;
            }
            break;
        case WhereOperands::WHERE_OP_LT:
            {
                BPlusTreeIterator bit = ind->begin();
                while (bit.available() && (DataOperands::compare(cif->columnType, bit.getKey(), wc.expr.val.data) < 0)){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                return res;
            }
            break;
        case WhereOperands::WHERE_OP_GE:
            {
                BPlusTreeIterator bit = ind->lowerBound(wc.expr.val.data);

                while (bit.available()){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                return res;
            }
            break;
        case WhereOperands::WHERE_OP_LE:
            {
                BPlusTreeIterator bit = ind->begin();
                while (bit.available() && (DataOperands::compare(cif->columnType, bit.getKey(), wc.expr.val.data) <= 0)){
                    res.push_back(RID(bit.getValue()));
                    bit.next();
                }

                return res;
            }
            break;
        default:
            return res;
            break;
    }
    return res;
}

void CrudHelper::solveForeignKey_delete(DatabaseManager *db, shared_ptr<TableInfo> tif, const vector<RID> &rids){
    for (RID r : rids){
        data_ptr dat = tif->dataFile->getData(r);
        tif->cvt->fromByteArray(dat);
        for (int i=0;i<tif->colNumbers;i++){
            auto cif = tif->colInfos[i];
            if (cif->referedBy.size() > 0){
                data_ptr key = tif->cvt->getRawData(i);
                for (auto c : cif->referedBy){
                    auto refTable = db->tablePool[c.first];
                    auto refCol = refTable->colInfoMapping[c.second];
                    vector<RID> newRes = refCol->indexTree->getRIDs(key);
                    solveForeignKey_delete(db, refTable, newRes);
                }
            }
        }

        tif->dataFile->deleteData(r);
        for (int i=0;i<tif->colNumbers;i++){
            auto cif = tif->colInfos[i];
            if (cif->useIndex){
                data_ptr key = tif->cvt->getRawData(i);
                cif->indexTree->remove(key, r.toInt());
            }
        }
    }
}