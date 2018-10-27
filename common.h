#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include "./filesystem/bufmanager/BufPageManager.h"
#include "./filesystem/fileio/FileManager.h"

using namespace std;

typedef shared_ptr<vector<unsigned char>> data_ptr;

#endif