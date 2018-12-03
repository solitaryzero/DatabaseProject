#ifndef STATEMENT_H
#define STATEMENT_H

#include "DatabaseManager.h"
#include "HelperClasses.h"
#include <cassert>

class Statement{
public:
    virtual void run(DatabaseManager *db) = 0;
};

class EmptyStatement : public Statement{
public:
    void run(DatabaseManager *db){
        // pass
    }
};

class ShowDatabaseStatement : public Statement{
public:
    void run(DatabaseManager *db);
};


class CreateDatabaseStatement : public Statement{
public:
    string dbName;

    CreateDatabaseStatement(string *name);
    void run(DatabaseManager *db);
};

class DropDatabaseStatement : public Statement{
public:
    string dbName;

    DropDatabaseStatement(string *name);
    void run(DatabaseManager *db);
};

class UseDatabaseStatement : public Statement{
public:
    string dbName;

    UseDatabaseStatement(string *name);
    void run(DatabaseManager *db);
};

class ShowTableStatement : public Statement{
public:
    void run(DatabaseManager *db);
};

class CreateTableStatement : public Statement{
public:
    string tbName;
    vector<Field> fieldList;

    CreateTableStatement(string *name, vector<Field> *fieldList);
    void run(DatabaseManager *db);
};

class DropTableStatement : public Statement{
public:
    string tbName;

    DropTableStatement(string *name);
    void run(DatabaseManager *db);
};

class DescTableStatement : public Statement{
public:
    string tbName;

    DescTableStatement(string *name);
    void run(DatabaseManager *db);
};

class CreateIndexStatement : public Statement{
public:
    string tbName;
    string colName;

    CreateIndexStatement(string *tbName, string *colName);
    void run(DatabaseManager *db);
};

class DropIndexStatement : public Statement{
public:
    string tbName;
    string colName;

    DropIndexStatement(string *tbName, string *colName);
    void run(DatabaseManager *db);
};

#endif