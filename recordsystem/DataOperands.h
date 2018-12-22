#ifndef DATAOPERANDS_H
#define DATAOPERANDS_H

#include "../common.h"

class DataOperands{
public:
    static const map<string, varTypes> nttMap;
    static map<string, varTypes> createMap();
    static string typeName(varTypes t);
    static varTypes nameToType(string s);
    static int compare(varTypes t, data_ptr left, data_ptr right);
    static data_ptr add(varTypes t, data_ptr left, data_ptr right);
    static int getTypeSize(varTypes t);
    static string toString(varTypes t, data_ptr p);
};

#endif