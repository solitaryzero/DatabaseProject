#ifndef HELPERCLASSES_H
#define HELPERCLASSES_H

#include <string>
#include "../common.h"
#include "../recordsystem/DataOperands.h"
using namespace std;

class Type{
public:
    Type(){
        this->t = varTypes::UNKNOWN_TYPE;
        this->size = -1;
    }
    Type(varTypes t){
        this->t = t;
        this->size = DataOperands::getTypeSize(t);
    }
    Type(varTypes t, int s) : t(t), size(s) {}

    varTypes t;
    int size;
};

class Field{
public:
    Field(string *ss, Type *tt, bool allowNull){
        this->name = *ss;
        delete ss;
        this->t = *tt;
        delete tt;
        this->allowNull = allowNull;
    }

    Field(vector<string> *ss){
        this->colNames = *ss;
        delete ss;
    }

    Field(string *scName, string *dtName, string *dcName){
        this->srcColName = *scName;
        delete scName;
        this->destTabName = *dtName;
        delete dtName;
        this->destColName = *dcName;
        delete dcName;
    }

    string name;
    Type t;
    bool allowNull;
    int mode;

    vector<string> colNames;

    string srcColName;
    string destTabName;
    string destColName;
};

class Value{
public:
    varTypes type;
    string raw;
    data_ptr data;

    Value();
    Value(varTypes type, data_ptr data);

    static data_ptr intToFloat(Value v);
    static data_ptr stringToDate(Value v);
    static data_ptr intToDecimal(Value v);
    static data_ptr floatToDecimal(Value v);
};

class ValueList{
public:
    vector<Value> values;
};

enum WhereOperands{
    WHERE_OP_EQ,
    WHERE_OP_NE,
    WHERE_OP_GE,
    WHERE_OP_LE,
    WHERE_OP_GT,
    WHERE_OP_LT,
    WHERE_OP_NOTNULL,
    WHERE_OP_ISNULL
};

class Column{
public:
    bool hasTableName;
    string tableName;
    string colName;

    Column();
    Column(string *colName);
    Column(string *tableName, string *colName);
};

class Expr{
public:
    ExprType type;
    Column col;
    Value val;

    Expr();
    Expr(Column *col);
    Expr(Value *val);
    Expr(Value val);
    Expr(ExprType type, Column *col, Value *val);
};

class WhereClause{
public:
    Column col;
    WhereOperands op;
    Expr expr;

    WhereClause(Column *col, WhereOperands op);
    WhereClause(Column *col, WhereOperands op, Expr* expr);

    void reverse();
};

class SetClause{
public:
    string colName;
    Value v;

    SetClause(string *colName, Value *v);
};

class Selector{
public:
    SelectorType type;
    vector<Column> cols;
};

class IntValue : public Value{
public:
    IntValue(string *raw);
};

class FloatValue : public Value{
public:
    FloatValue(string *raw);
};

class StringValue : public Value{
public:
    StringValue(string *raw);
};

class NullValue : public Value{
public:
    NullValue();
};

class WildSelector : public Selector{
public:
    WildSelector();
};

class ColumnSelector : public Selector{
public:
    ColumnSelector(vector<Column> *cols);
};

class AggSelector : public Selector{
public:
    AggSelector(SelectorType type, Column *col);
    AggSelector(SelectorType type);
};

#endif