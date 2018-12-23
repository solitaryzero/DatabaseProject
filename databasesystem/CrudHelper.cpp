#include "CrudHelper.h"

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
    assert((minCount == CrudHelper::failed) || ((unsigned int)minCount == _res.size()));
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
    if ((cif->useIndex == 0) || (wc.expr.type == ExprType::COL_EXPR)){
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

bool CrudHelper::getTableName(DatabaseManager *db, Column &col, vector<string> &tables){
    if (col.hasTableName){
        for (string s : tables){
            if (s == col.tableName){
                return true;
            } else {
                cout << "[Error] Table " << col.tableName << " doesn't exist!\n";
                return false;
            }
        }
    }

    for (string s : tables){
        if (db->tablePool.find(s) != db->tablePool.end()){
            auto tif = db->tablePool[s];
            if (tif->colIndex.find(col.colName) != tif->colIndex.end()){
                if (col.hasTableName){
                    cout << "[Error] Duplicate table possibilities for column " << col.colName << ".\n";
                    return false;
                } else {
                    col.hasTableName = true;
                    col.tableName = tif->tableName;
                }
            }
        }
    }

    if (!col.hasTableName){
        cout << "[Error] No table matches column " << col.colName << ".\n";
        return false;
    }

    return true;
}

WhereClause CrudHelper::genConstraint(string tableName, string columnName, Value v){
    string *tname = new string(tableName);
    string *cname = new string(columnName);
    Column *c = new Column(tname, cname);
    Expr *e = new Expr(v);
    return WhereClause(c, WhereOperands::WHERE_OP_EQ, e);
}