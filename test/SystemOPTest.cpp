#include "common.h"
#include "./databasesystem/Glue.h"

int main(){
    system("rm -r ./data/*");
    dbm = new DatabaseManager();
    parseSQL("./test.sql");
    delete(dbm);
}