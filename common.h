#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include <memory.h>
#include "./filesystem/bufmanager/BufPageManager.h"
#include "./filesystem/fileio/FileManager.h"

using namespace std;

#define TABLENAMELENGTH 256
typedef shared_ptr<vector<unsigned char>> data_ptr;

class DataContainer{
public:
    static data_ptr genDataContainer(int size){
        data_ptr res = make_shared<vector<unsigned char>>(size);
        return res;
    }
};

#endif