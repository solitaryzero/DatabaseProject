#include "../common.h"
#include "../json11/json11.hpp"

class TableInfo{
public:
    TableInfo();
    ~TableInfo();

    string tableName;
    vector<string> colNames;
};