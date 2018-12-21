#ifndef STATEMENT_H
#define STATEMENT_H

#include "DatabaseManager.h"
#include "HelperClasses.h"
#include <cassert>

class Statement{
public:
    virtual void run(DatabaseManager *db) = 0;
    virtual ~Statement() = 0;
};

#endif