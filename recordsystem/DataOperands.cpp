#include "DataOperands.h"

map<string, varTypes> DataOperands::createMap(){
    map<string, varTypes> m = {
        {"int", varTypes::INT_TYPE},
        {"float", varTypes::FLOAT_TYPE},
        {"char", varTypes::CHAR_TYPE},
        {"varchar", varTypes::VARCHAR_TYPE},
        {"date", varTypes::DATE_TYPE},
        {"decimal", varTypes::DECIMAL_TYPE}
    };
    return m;
}

const map<string, varTypes> DataOperands::nttMap = DataOperands::createMap();

string DataOperands::typeName(varTypes t){
    switch(t){
        case varTypes::INT_TYPE:
            return "int";
        case varTypes::FLOAT_TYPE:
            return "float";
        case varTypes::CHAR_TYPE:
            return "char";
        case varTypes::VARCHAR_TYPE:
            return "varchar";
        case varTypes::DATE_TYPE:
            return "date";
        case varTypes::DECIMAL_TYPE:
            return "decimal";
        default:
            assert(false);
            return "";
    }
}

varTypes DataOperands::nameToType(string s){
    auto res = nttMap.find(s);
    assert(res != nttMap.end());
    return res->second;
}

int DataOperands::compare(varTypes t, data_ptr left, data_ptr right){
    switch(t){
        case varTypes::INT_TYPE:
        {
            return (*(int*)(left->data())-*(int*)(right->data()));
        }
        case varTypes::FLOAT_TYPE:
        {
            float res = (*(float*)(left->data())-*(float*)(right->data()));
            if (res < 0){
                return -1;
            } else if (res == 0) {
                return 0;
            } else {
                return 1;
            }
            break;
        }
        case varTypes::CHAR_TYPE:
        case varTypes::VARCHAR_TYPE:
        {
            unsigned int len = min(left->size(), right->size());
            for (unsigned int i=0;i<len;i++){
                if (left->data()[i] < right->data()[i]) return -1; 
                if (left->data()[i] > right->data()[i]) return 1; 
            }
            if (left->size() > len) return 1;
            if (right->size() > len) return -1;
            return 0;
            break;
        }
        case varTypes::DECIMAL_TYPE:
        {
            int l1 = (*(int*)(left->data()));
            int l2 = (*(int*)(left->data()+sizeof(int)));
            int r1 = (*(int*)(right->data()));
            int r2 = (*(int*)(right->data()+sizeof(int)));
            if (l1 < r1) return -1;
            if (l1 > r1) return 1;
            if (l2 < r2) return -1;
            if (l2 > r2) return 1;
            return 0;
            break;
        }
        case varTypes::DATE_TYPE:
        {
            int lyear = (*(int*)(left->data()));
            int ryear = (*(int*)(right->data()));
            int lmonth = (*(int*)(left->data()+sizeof(int)));
            int rmonth = (*(int*)(right->data()+sizeof(int)));
            int lday = (*(int*)(left->data()+sizeof(int)*2));
            int rday = (*(int*)(right->data()+sizeof(int)*2));
            if (lyear < ryear) return -1;
            if (lyear > ryear) return 1;
            if (lmonth < rmonth) return -1;
            if (lmonth > rmonth) return 1;
            if (lday < rday) return -1;
            if (lday > rday) return 1;
        }
        default:
            assert(false);
    }
    return -1;
}

data_ptr DataOperands::add(varTypes t, data_ptr left, data_ptr right){
    data_ptr cont;
    switch(t){
        case varTypes::INT_TYPE:
        {
            int res = (*(int*)(left->data())+*(int*)(right->data()));
            cont = DataContainer::genDataContainer(sizeof(int));
            *(int*)(cont->data()) = res;
            return cont;
        }
        case varTypes::FLOAT_TYPE:
        {
            float res = (*(float*)(left->data())+*(float*)(right->data()));
            cont = DataContainer::genDataContainer(sizeof(float));
            *(float*)(cont->data()) = res;
            return cont;
        }
        default:
            assert(false);
    }
    return nullptr;
}

int DataOperands::getTypeSize(varTypes t){
    switch (t){
        case varTypes::INT_TYPE:
            return sizeof(int);
        case varTypes::FLOAT_TYPE:
            return sizeof(float);
        case varTypes::DECIMAL_TYPE:
            return sizeof(int)*2;
        case varTypes::DATE_TYPE:
            return sizeof(int)*3;
        default:
            return -1;
    }
}