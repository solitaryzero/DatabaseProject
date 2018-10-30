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
    if ((this->columnType == varTypes::CHAR_TYPE) || (this->columnType == varTypes::VARCHAR_TYPE)){
        this->size = j["size"].int_value();
    } else {
        this->size = DataOperands::getTypeSize(this->columnType);
    }
}

ColumnInfo::ColumnInfo(TableInfo *t, string colName, varTypes colType, int siz){
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
}

ColumnInfo::~ColumnInfo(){
    
}

Json ColumnInfo::infoToJson(){
    Json res = Json::object{
        {"name", this->columnName},
        {"type", this->columnTypeName},
        {"size", this->size}
    };
    return res;
}

string ColumnInfo::toJsonDump(){
    return infoToJson().dump();
}