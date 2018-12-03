#ifndef HELPERCLASSES_H
#define HELPERCLASSES_H

#include <string>
#include "../common.h"
#include "../recordsystem/DataOperands.h"
using namespace std;

class Type{
public:
    Type(){
        this->t = varTypes::UNKNOWN_TYPE;
        this->size = -1;
    }
    Type(varTypes t){
        this->t = t;
        this->size = DataOperands::getTypeSize(t);
    }
    Type(varTypes t, int s) : t(t), size(s) {}

    varTypes t;
    int size;
};

class Field{
public:
    Field(string *ss, Type *tt, bool allowNull){
        this->name = *ss;
        delete ss;
        this->t = *tt;
        delete tt;
        this->allowNull = allowNull;
    }

    Field(string *ss){
        this->name = *ss;
        delete ss;
        this->allowNull = false;
    }

    string name;
    Type t;
    bool allowNull;
    int mode;
};

#endif