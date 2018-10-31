#include "RecordConverter.h"

RecordConverter::RecordConverter(TableInfo* t){
    this->tinfo = t;
    this->fixedLength = 0;
    int unfixedIndex = 0;
    for (int i=0;i<this->tinfo->colNumbers;i++){
        if (this->tinfo->colInfos[i]->isFixed){
            this->colOffset.push_back(this->fixedLength);
            this->fixedLength += this->tinfo->colInfos[i]->size;
        } else {
            unfixedIndex++;
            this->colOffset.push_back(-unfixedIndex);
        }
    }
    this->values.resize(this->tinfo->colNumbers);
    for (int i=0;i<this->tinfo->colNumbers;i++){
        this->values[i] = nullptr;
    }
}

RecordConverter::~RecordConverter(){

}

data_ptr RecordConverter::toByteArray(){
    data_ptr res = make_shared<vector<unsigned char>>();
    res->push_back(0); //状态位A
    res->push_back(0); //状态位B
    data_ptr flen = DataContainer::genShortData((short)this->fixedLength);
    DataContainer::concat(res, flen);
    for (unsigned int i=0;i<this->values.size();i++){
        if (this->tinfo->colInfos[i]->isFixed){
            if (this->values[i] != nullptr){
                DataContainer::concat(res, this->values[i]);
            } else {
                data_ptr tmp = DataContainer::genDataContainer(this->tinfo->colInfos[i]->size);
                DataContainer::concat(res, tmp);
            }
        }
    }
    data_ptr fcoln = DataContainer::genShortData((short)this->tinfo->fixedColNumbers);
    DataContainer::concat(res, fcoln);
    int bitmapSize = (this->tinfo->colNumbers+7)/8;
    data_ptr bitmap = DataContainer::genDataContainer(bitmapSize);
    for (unsigned int i=0;i<this->values.size();i++){
        if (this->values[i] == nullptr){
            bitmap->data()[i/8] |= (1 << (7-(i%8))); 
        }
    }
    data_ptr ufcoln = DataContainer::genShortData((short)this->tinfo->unfixedColNumbers);
    DataContainer::concat(res, ufcoln);
    int unfixedOffset = 0;
    for (unsigned int i=0;i<this->values.size();i++){
        if (!(this->tinfo->colInfos[i]->isFixed)){
            data_ptr tmp = DataContainer::genShortData((short)unfixedOffset);
            DataContainer::concat(res, tmp);
            if (this->values[i] != nullptr){
                unfixedOffset += this->values[i]->size();
            }
        }
    }

    for (unsigned int i=0;i<this->values.size();i++){
        if (!(this->tinfo->colInfos[i]->isFixed)){
            if (this->values[i] != nullptr){
                DataContainer::concat(res, this->values[i]);
            }
        }
    }

    return res;
}

void RecordConverter::fromNameValueMap(map<string, data_ptr> vmap){
    for (int i=0;i<this->tinfo->colNumbers;i++){
        this->values[i] = nullptr;
    }

    for (auto it=vmap.begin();it!=vmap.end();it++){
        if (this->tinfo->colInfoMapping[it->first]->columnType == varTypes::CHAR_TYPE){
            this->values[this->tinfo->colIndex[it->first]] = it->second;
            this->values[this->tinfo->colIndex[it->first]]->resize(this->tinfo->colInfoMapping[it->first]->size, ' ');
        } else {
            this->values[this->tinfo->colIndex[it->first]] = it->second;
        }
    }
}

void RecordConverter::fromIndexValueMap(map<int, data_ptr> vmap){
    for (int i=0;i<this->tinfo->colNumbers;i++){
        this->values[i] = nullptr;
    }

    for (auto it=vmap.begin();it!=vmap.end();it++){
        this->values[it->first] = it->second;
    }
}

void RecordConverter::fromByteArray(data_ptr dat){
    int offset = 2;
    short flen = *((short*)(dat->data()+offset));
    offset += 2;
    for (unsigned int i=0;i<this->values.size();i++){
        if (this->tinfo->colInfos[i]->isFixed){
            int s = this->tinfo->colInfos[i]->size;
            data_ptr tmp = DataContainer::genDataContainer(s);
            memcpy(tmp->data(), dat->data()+offset, s);
            this->values[i] = tmp;
            offset += s;
        }
    }
    assert(offset == (int)(flen+4));

    short fcoln = *((short*)(dat->data()+offset));
    offset += 2;
    assert((int)(fcoln) == this->tinfo->fixedColNumbers);

    int bitmapSize = (this->tinfo->colNumbers+7)/8;
    int bitmapOffset = offset;
    offset += bitmapSize;
    
    short ufcoln = *((short*)(dat->data()+offset));
    offset += 2;
    assert((int)(ufcoln) == this->tinfo->unfixedColNumbers);

    int unfixedDataBase = offset + 2*this->tinfo->unfixedColNumbers;
    for (unsigned int i=0;i<this->values.size();i++){
        if (!(this->tinfo->colInfos[i]->isFixed)){
            assert(offset < unfixedDataBase);
            int dOff = *((short*)(dat->data()+offset));
            offset += 2;
            if (offset < unfixedDataBase){
                int nextOff = *((short*)(dat->data()+offset));
                int s = nextOff-dOff;
                data_ptr tmp = DataContainer::genDataContainer(s);
                if (s != 0){
                    memcpy(tmp->data(), dat->data()+unfixedDataBase+dOff, s);
                }
                this->values[i] = tmp;
            } else {
                int s = dat->size()-dOff;
                data_ptr tmp = DataContainer::genDataContainer(s);
                if (s != 0){
                    memcpy(tmp->data(), dat->data()+unfixedDataBase+dOff, s);
                }
                this->values[i] = tmp;
            }
        }
    }
    assert(offset == unfixedDataBase);

    for (unsigned int i=0;i<this->values.size();i++){
        unsigned char t = *(dat->data()+bitmapOffset+(i/8));
        if ((t & (1 << (7-(i%8)))) != 0){
            this->values[i] = nullptr;
        }
    }
}

data_ptr RecordConverter::getRawData(string colName){
    return this->values[this->tinfo->colIndex[colName]];
}

data_ptr RecordConverter::getRawData(int colIndex){
    return this->values[colIndex];
}

void RecordConverter::setRawData(string colName, data_ptr dat){
    this->values[this->tinfo->colIndex[colName]] = dat;
}

void RecordConverter::setRawData(int colIndex, data_ptr dat){
    this->values[colIndex] = dat;
}

int RecordConverter::getInt(string colName){
    assert(this->tinfo->colInfoMapping[colName]->columnType == varTypes::INT_TYPE);
    return *(int*)(this->values[this->tinfo->colIndex[colName]]->data());
}

int RecordConverter::getInt(int colIndex){
    assert(this->tinfo->colInfos[colIndex]->columnType == varTypes::INT_TYPE);
    return *(int*)(this->values[colIndex]->data());
}

void RecordConverter::setInt(string colName, int dat){
    assert(this->tinfo->colInfoMapping[colName]->columnType == varTypes::INT_TYPE);
    *(int*)(this->values[this->tinfo->colIndex[colName]]->data()) = dat;
}

void RecordConverter::setInt(int colIndex, int dat){
    assert(this->tinfo->colInfos[colIndex]->columnType == varTypes::INT_TYPE);
    *(int*)(this->values[colIndex]->data()) = dat;
}

float RecordConverter::getFloat(string colName){
    assert(this->tinfo->colInfoMapping[colName]->columnType == varTypes::FLOAT_TYPE);
    return *(float*)(this->values[this->tinfo->colIndex[colName]]->data());
}

float RecordConverter::getFloat(int colIndex){
    assert(this->tinfo->colInfos[colIndex]->columnType == varTypes::FLOAT_TYPE);
    return *(float*)(this->values[colIndex]->data());
}

void RecordConverter::setFloat(string colName, float dat){
    assert(this->tinfo->colInfoMapping[colName]->columnType == varTypes::FLOAT_TYPE);
    *(float*)(this->values[this->tinfo->colIndex[colName]]->data()) = dat;
}

void RecordConverter::setFloat(int colIndex, float dat){
    assert(this->tinfo->colInfos[colIndex]->columnType == varTypes::FLOAT_TYPE);
    *(float*)(this->values[colIndex]->data()) = dat;
}

string RecordConverter::getChar(string colName, bool strip){
    return getChar(this->tinfo->colIndex[colName], strip);
}

string RecordConverter::getChar(int colIndex, bool strip){
    assert((this->tinfo->colInfos[colIndex]->columnType == varTypes::CHAR_TYPE) || (this->tinfo->colInfos[colIndex]->columnType == varTypes::VARCHAR_TYPE));
    string res((char*)(this->values[colIndex]->data()), this->values[colIndex]->size());
    if (strip && (this->tinfo->colInfos[colIndex]->columnType == varTypes::CHAR_TYPE)){
        while(res.back() == ' '){
            res.pop_back();
        }
    }
    return res;
}

void RecordConverter::setChar(string colName, string dat, bool padding){
    setChar(this->tinfo->colIndex[colName], dat, padding);
}

void RecordConverter::setChar(int colIndex, string dat, bool padding){
    varTypes type = this->tinfo->colInfos[colIndex]->columnType;
    assert((type == varTypes::CHAR_TYPE) || (type == varTypes::VARCHAR_TYPE));
    data_ptr val = DataContainer::genStringData(dat);
    if ((type == varTypes::CHAR_TYPE) && padding){
        val->resize(this->tinfo->colInfos[colIndex]->size, ' ');
    }
    this->values[colIndex] = val;
}

void RecordConverter::setNull(string colName){
    setNull(this->tinfo->colIndex[colName]);
}

void RecordConverter::setNull(int colIndex){
    this->values[colIndex] = nullptr;
}

bool RecordConverter::isNull(string colName){
    return (this->values[this->tinfo->colIndex[colName]] == nullptr);
}

bool RecordConverter::isNull(int colIndex){
    return (this->values[colIndex] == nullptr);
}