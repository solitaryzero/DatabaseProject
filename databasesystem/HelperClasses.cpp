#include "HelperClasses.h"

data_ptr Value::intToFloat(Value v){
    data_ptr newData = DataContainer::genFloatData(float(*(int*)(v.data->data())));
    return newData;
}

data_ptr Value::stringToDate(Value v){
    vector<int> maxDayPerMonth = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int year, month, day;
    if (sscanf(v.raw.c_str(), "%d-%d-%d", &year, &month, &day) < 3){
        return nullptr;
    }
    if (year < 0) return nullptr;
    if ((month < 1) || (month > 12)) return nullptr;
    int maxDay = maxDayPerMonth[month];
    if ((month == 2) && (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))){
        maxDay++;
    }
    if ((day < 1) || (day > maxDay)) return nullptr;

    return DataContainer::genDateData(year, month, day);
}

data_ptr Value::intToDecimal(Value v){
    int x = *(int*)(v.data->data());
    return DataContainer::genDecimalData(x, 0);
}

data_ptr Value::floatToDecimal(Value v){
    int x, y;
    sscanf(v.raw.c_str(), "%d.%d", &x, &y);
    return DataContainer::genDecimalData(x, y);
}

IntValue::IntValue(string *raw){
    this->type = varTypes::INT_TYPE;
    this->raw = *raw;
    this->data = DataContainer::genIntData(stoi(*raw));
    delete raw;
}

FloatValue::FloatValue(string *raw){
    this->type = varTypes::FLOAT_TYPE;
    this->raw = *raw;
    this->data = DataContainer::genFloatData(stof(*raw));
    delete raw;
}

StringValue::StringValue(string *raw){
    this->type = varTypes::CHAR_TYPE;
    this->raw = *raw;
    this->data = DataContainer::genStringData(*raw);
    delete raw;
}

NullValue::NullValue(){
    this->type = varTypes::UNKNOWN_TYPE;
    this->raw = "";
    this->data = nullptr;
}

Column::Column(){
    this->hasTableName = false;
    this->tableName = "";
    this->colName = "";
}

Column::Column(string *tableName, string *colName){
    this->hasTableName = true;
    this->tableName = *tableName;
    this->colName = *colName;
    delete tableName;
    delete colName;
}

Column::Column(string *colName){
    this->hasTableName = false;
    this->tableName = "";
    this->colName = *colName;
    delete colName;
}

Expr::Expr(){
    this->type = ExprType::VALUE_EXPR;
    this->col = Column();
    this->val = NullValue();
}

Expr::Expr(Column *col){
    this->type = ExprType::COL_EXPR;
    this->col = *col;
    this->val = NullValue();
    delete col;
}

Expr::Expr(Value *val){
    this->type = ExprType::VALUE_EXPR;
    this->col = Column();
    this->val = *val;
    delete val;
}

Expr::Expr(ExprType type, Column *col, Value *val){
    this->type = type;
    this->col = *col;
    this->val = *val;
    delete col;
    delete val;
}

WhereClause::WhereClause(Column *col, WhereOperands op){
    this->col = *col;
    this->op = op;
    this->expr = Expr();
    delete col;
}

WhereClause::WhereClause(Column *col, WhereOperands op, Expr* expr){
    this->col = *col;
    this->op = op;
    this->expr = *expr;
    delete col;
    delete expr;
}

SetClause::SetClause(string *colName, Value *v){
    this->colName = *colName;
    this->v = *v;

    delete colName;
    delete v;
}

WildSelector::WildSelector(){
    this->type = SelectorType::WILD_SELECTOR;
}

ColumnSelector::ColumnSelector(vector<Column> *cols){
    this->type = SelectorType::COL_SELECTOR;
    this->cols = *cols;
    delete cols;
}