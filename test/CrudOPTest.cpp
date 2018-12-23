#include "common.h"
#include "./databasesystem/Glue.h"

int crudInit(){
    if (system("mkdir data") == -1) return -1;
    if (system("rm -r ./data/*") == -1) return -1;
    parseSQL("../dataset_small/create.sql");
    cout << "[Info] Create complete.\n";
    parseSQL("../dataset_small/customer.sql");
    parseSQL("../dataset_small/book.sql");
    parseSQL("../dataset_small/website.sql");
    parseSQL("../dataset_small/price.sql");
    //parseSQL("../dataset_small/orders.sql");
    cout << "[Info] Insert complete.\n";
    return 0;
}

void crudTest(){
    parseSQL("../test3.sql");
    parseSQL(stdin);
}

int main(){
    dbm = new DatabaseManager();
    crudInit();
    crudTest();
    delete dbm;
    return 0;
}