#ifndef SYSTEMSTATEMENT_H
#define SYSTEMSTATEMENT_H

#include "Statement.h"

class EmptyStatement : public Statement{
public:
    void run(DatabaseManager *db){
        // pass
    }
    ~EmptyStatement();
};

class ShowDatabaseStatement : public Statement{
public:
    void run(DatabaseManager *db);
    ~ShowDatabaseStatement();
};


class CreateDatabaseStatement : public Statement{
public:
    string dbName;

    CreateDatabaseStatement(string *name);
    ~CreateDatabaseStatement();
    void run(DatabaseManager *db);
};

class DropDatabaseStatement : public Statement{
public:
    string dbName;

    DropDatabaseStatement(string *name);
    ~DropDatabaseStatement();
    void run(DatabaseManager *db);
};

class UseDatabaseStatement : public Statement{
public:
    string dbName;

    UseDatabaseStatement(string *name);
    ~UseDatabaseStatement();
    void run(DatabaseManager *db);
};

class ShowTableStatement : public Statement{
public:
    void run(DatabaseManager *db);
    ~ShowTableStatement();
};

class CreateTableStatement : public Statement{
public:
    string tbName;
    vector<Field> fieldList;

    CreateTableStatement(string *name, vector<Field> *fieldList);
    ~CreateTableStatement();
    void run(DatabaseManager *db);
};

class DropTableStatement : public Statement{
public:
    string tbName;

    DropTableStatement(string *name);
    ~DropTableStatement();
    void run(DatabaseManager *db);
};

class DescTableStatement : public Statement{
public:
    string tbName;

    DescTableStatement(string *name);
    ~DescTableStatement();
    void run(DatabaseManager *db);
};

class CreateIndexStatement : public Statement{
public:
    string tbName;
    string colName;

    CreateIndexStatement(string *tbName, string *colName);
    ~CreateIndexStatement();
    void run(DatabaseManager *db);
};

class DropIndexStatement : public Statement{
public:
    string tbName;
    string colName;

    DropIndexStatement(string *tbName, string *colName);
    ~DropIndexStatement();
    void run(DatabaseManager *db);
};

#endif