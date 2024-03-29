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

void TableInfo::addNewColumn(string colName, varTypes colType, int siz, int useIndex){
    shared_ptr<ColumnInfo> c = make_shared<ColumnInfo>(this, colName, colType, siz, useIndex);
    this->colInfos.push_back(c);
    this->colIndex.insert({colName, this->colInfos.size()-1});
    this->colInfoMapping.insert({colName, c});
    if (colType == varTypes::VARCHAR_TYPE){
        this->unfixedColNumbers++;
    } else {
        this->fixedColNumbers++;
    }
    this->colNumbers++;
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
    if (this->dataFile != nullptr){
        this->dataFile->closeFile();
    }
}

void TableInfo::genConverter(){
    this->cvt = make_shared<RecordConverter>(this);
}

int TableInfo::getFixedLength(){
    int sum = 0;
    for (int i=0;i<colNumbers;i++){
        if (this->colInfos[i]->isFixed){
            sum += this->colInfos[i]->size;
        }
    }
    return sum;
}

int TableInfo::getFixedRecordLength(){
    if (this->unfixedColNumbers != 0){
        return -1;
    }
    int sum = getFixedLength();
    return sum+4+2+2+((this->fixedColNumbers+7)/8);
}

void TableInfo::showTableInfo(){
    printf("=============\n");
    printf("[Info] Table name: %-30s\n", this->tableName.c_str());
    printf("[Info] %d columns in total:\n", this->colNumbers);
    printf("Name        Type        Size        Indexed     AllowNull   Primary\n");
    for (int i=0;i<this->colNumbers;i++){
        printf("%-12s%-12s%-12d%-12d%-12d%-12d\n", this->colInfos[i]->columnName.c_str(), this->colInfos[i]->columnTypeName.c_str(), this->colInfos[i]->size,
        this->colInfos[i]->useIndex, this->colInfos[i]->allowNull, this->colInfos[i]->isPrimary);
    }

    printf("-------------\n");
    for (int i=0;i<this->colNumbers;i++){
        if (this->colInfos[i]->hasForeign){
            printf("Column %s is foreign key, it refers to %s.%s\n", this->colInfos[i]->columnName.c_str(), this->colInfos[i]->foreignTableName.c_str(), this->colInfos[i]->foreignColumnName.c_str());
        }

        if (this->colInfos[i]->referedBy.size() > 0){
            printf("Column %s is used as foreign key by: ", this->colInfos[i]->columnName.c_str());
            for (unsigned int j=0;j<this->colInfos[i]->referedBy.size();j++){
                printf("(%s.%s) ", this->colInfos[i]->referedBy[j].first.c_str(), this->colInfos[i]->referedBy[j].second.c_str());
            }
            printf("\n");
        }
    }

    printf("=============\n");
}

void TableInfo::openDataFile(){
    string dataFileName = this->tableName+".tbdata";
    if (this->unfixedColNumbers == 0){
        shared_ptr<FixedRecordFile> p = make_shared<FixedRecordFile>(dataFileName);
        p->setRecordLength(this->getFixedRecordLength());
        this->dataFile = p;
    } else {
        this->dataFile = make_shared<UnfixedRecordFile>(dataFileName);
    }
    this->genConverter();
}