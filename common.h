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
typedef shared_ptr<vector<unsigned char>> data_ptr;

enum class varTypes {
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    VARCHAR_TYPE,
    DATE_TYPE,
    DECIMAL_TYPE
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
};

#endif