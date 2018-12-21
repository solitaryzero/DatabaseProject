#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <memory.h>
#include <utility>
#include "./filesystem/bufmanager/BufPageManager.h"
#include "./filesystem/fileio/FileManager.h"
#include "./json11/json11.hpp"

using namespace std;
using namespace json11;

#define TABLENAMELENGTH 256

#define MAXINDEXRECPERPAGE 650
#define MAXRECPEROVERFLOWPAGE 2000

#define FIELD_UNKNOWN 0
#define FIELD_COMMON 1
#define FIELD_NOTNULL 2
#define FIELD_PRIMARY 3
#define FIELD_FOREIGN 4

//for b-plus tree test
/*
#define MAXINDEXRECPERPAGE 5
#define MAXRECPEROVERFLOWPAGE 2
*/

typedef shared_ptr<vector<unsigned char>> data_ptr;

enum class varTypes {
    UNKNOWN_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    VARCHAR_TYPE,
    DATE_TYPE,
    DECIMAL_TYPE
};

enum class ExprType{
    COL_EXPR,
    VALUE_EXPR
};

enum class SelectorType{
    WILD_SELECTOR,
    COL_SELECTOR
};

class date{
public:
    int year;
    int month;
    int day;
};

class decimal{
public:
    int integer;
    int remainder;
};

class DataContainer{
public:
    static data_ptr genDataContainer(int size){
        data_ptr res = make_shared<vector<unsigned char>>(size);
        memset(res->data(), 0, size);
        return res;
    }

    static void concat(data_ptr a, data_ptr b){
        int sizea = a->size();
        int sizeb = b->size();
        a->resize(sizea+sizeb);
        memcpy(a->data()+sizea, b->data(), sizeb);
    }

    static data_ptr genIntData(int v){
        data_ptr res = genDataContainer(sizeof(int));
        *(int*)(res->data()) = v;
        return res;
    }

    static data_ptr genFloatData(float v){
        data_ptr res = genDataContainer(sizeof(float));
        *(float*)(res->data()) = v;
        return res;
    }

    static data_ptr genShortData(short v){
        data_ptr res = genDataContainer(sizeof(short));
        *(short*)(res->data()) = v;
        return res;
    }

    static data_ptr genStringData(string s){
        data_ptr res = genDataContainer(s.length());
        for (unsigned int i=0;i<s.length();i++){
            res->data()[i] = (unsigned char)s[i];
        }
        return res;
    }

    static data_ptr genDecimalData(int i, int r){
        data_ptr res = genDataContainer(sizeof(int)*2);
        *(int*)(res->data()) = i;
        *(int*)(res->data()+sizeof(int)) = r;
        return res;
    }

    static data_ptr genDateData(int year, int month, int day){
        data_ptr res = genDataContainer(sizeof(int)*3);
        *(int*)(res->data()) = year;
        *(int*)(res->data()+sizeof(int)) = month;
        *(int*)(res->data()+sizeof(int)) = day;
        return res;
    }
};

#endif