#include "common.h"
#include "./databasesystem/Glue.h"

int main(){
    if (system("mkdir data") == -1) return -1;
    if (system("rm -r ./data/*") == -1) return -1;
    dbm = new DatabaseManager();
    parseSQL("../test.sql");
    delete dbm;
    return 0;
}