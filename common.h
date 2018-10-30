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
};

#endif