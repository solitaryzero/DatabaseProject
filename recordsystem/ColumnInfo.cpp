#include "ColumnInfo.h"

ColumnInfo::ColumnInfo(TableInfo *t, string rawJson){
    this->tabInfo = t;
    string err;
    Json j = Json::parse(rawJson, err);
    this->columnName = j["name"].string_value();
    this->columnTypeName = j["type"].string_value();
    this->columnType = DataOperands::nameToType(this->columnTypeName);
    if (this->columnType == varTypes::VARCHAR_TYPE){
        this->isFixed = false;
    } else {
        this->isFixed = true;
    }
    if ((this->columnType == varTypes::CHAR_TYPE) || (this->columnType == varTypes::VARCHAR_TYPE) || (this->columnType == varTypes::INT_TYPE)){
        this->size = j["size"].int_value();
    } else {
        this->size = DataOperands::getTypeSize(this->columnType);
    }
    this->showLength = j["showLength"].int_value();
    this->useIndex = j["useIndex"].int_value();
    this->allowNull = j["allowNull"].bool_value();
    this->isPrimary = j["isPrimary"].bool_value();
    this->hasForeign = j["hasForeign"].bool_value();
    this->foreignTableName = j["foreignTableName"].string_value();
    this->foreignColumnName = j["foreignColumnName"].string_value();

    if (this->useIndex == 1){
        this->indexTree = make_shared<BPlusTree>(t->tableName, this->columnName, this->columnType);
    }
}

ColumnInfo::ColumnInfo(TableInfo *t, string colName, varTypes colType, int siz, int useIndex){
    this->tabInfo = t;
    this->columnName = colName;
    this->columnTypeName = DataOperands::typeName(colType);
    this->columnType = colType;
    if (this->columnType == varTypes::VARCHAR_TYPE){
        this->isFixed = false;
    } else {
        this->isFixed = true;
    }
    this->size = siz;
    this->useIndex = useIndex;
    if (this->useIndex == 1){
        this->indexTree = make_shared<BPlusTree>(t->tableName, this->columnName, this->columnType);
    }
}

ColumnInfo::~ColumnInfo(){
    if (this->indexTree != nullptr){
        this->indexTree->closeIndex();
        this->indexTree = nullptr;
    }
}

Json ColumnInfo::infoToJson(){
    Json res = Json::object{
        {"name", this->columnName},
        {"type", this->columnTypeName},
        {"size", this->size},
        {"showLength", this->showLength},
        {"useIndex", this->useIndex},
        {"allowNull", this->allowNull},
        {"isPrimary", this->isPrimary},
        {"hasForeign", this->hasForeign},
        {"foreignTableName", this->foreignTableName},
        {"foreignColumnName", this->foreignColumnName}
    };
    return res;
}

string ColumnInfo::toJsonDump(){
    return infoToJson().dump();
}

void ColumnInfo::setPrimary(bool para){
    if (!para){
        this->isPrimary = false;
        return;
    }

    this->isPrimary = true;
    if (this->useIndex == 0){
        this->useIndex = 1;
        this->indexTree = make_shared<BPlusTree>(this->tabInfo->tableName, this->columnName, this->columnType);
    }
}