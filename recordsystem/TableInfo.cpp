#include "TableInfo.h"

TableInfo::TableInfo(string tableName, bool newTable){
    if (!newTable){
        string fullName = tableName+".tbinfo";
        this->infoFileName = fullName;
        this->fs.open(fullName, ios::in);
        string line;
        getline(this->fs, line);
        string err;
        Json j = Json::parse(line, err);
        this->tableName = j["name"].string_value();
        this->colNumbers = j["coln"].int_value();
        this->fixedColNumbers = j["fixcoln"].int_value();
        this->unfixedColNumbers = j["unfixcoln"].int_value();

        for (int i=0;i<this->colNumbers;i++){
            getline(this->fs, line);
            shared_ptr<ColumnInfo> c = make_shared<ColumnInfo>(this, line);
            this->colInfos.push_back(c);
            this->colIndex.insert({c->columnName, i});
            this->colInfoMapping.insert({c->columnName, c});
        }
        this->fs.close();
    } else {
        string fullName = tableName+".tbinfo";
        this->infoFileName = fullName;
        this->tableName = tableName;
        this->colNumbers = 0;
        this->fixedColNumbers = 0;
        this->unfixedColNumbers = 0;
    }
}

TableInfo::~TableInfo(){
    writeBack();
}

void TableInfo::addNewColumn(string colName, varTypes colType, int siz){
    shared_ptr<ColumnInfo> c = make_shared<ColumnInfo>(this, colName, colType, siz);
    this->colInfos.push_back(c);
    this->colIndex.insert({colName, this->colInfos.size()-1});
    this->colInfoMapping.insert({colName, c});
}

string TableInfo::infoToString(){
    Json res = Json::object{
        {"name", this->tableName},
        {"coln", this->colNumbers},
        {"fixcoln", this->fixedColNumbers},
        {"unfixcoln", this->unfixedColNumbers}
    };
    return res.dump();
}

void TableInfo::writeBack(){
    this->fs.open(this->infoFileName, ios::out);
    this->fs << infoToString() << "\n";
    for (int i=0;i<this->colNumbers;i++){
        this->fs << this->colInfos[i]->toJsonDump() << "\n";
    }
    this->fs.close();
}