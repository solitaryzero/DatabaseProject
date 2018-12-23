#include "common.h"
#include "./databasesystem/Glue.h"

int main(){
    if (system("mkdir data") == -1) return -1;
    if (system("rm -r ./data/*") == -1) return -1;
    dbm = new DatabaseManager();
    //parseSQL("../test2.sql");
    parseSQL("../dataset_small/create.sql");
    cout << "[Info] Create complete.\n";
    parseSQL("../dataset_small/customer.sql");
    parseSQL("../dataset_small/book.sql");
    parseSQL("../dataset_small/website.sql");
    parseSQL("../dataset_small/price.sql");
    parseSQL("../dataset_small/orders.sql");
    cout << "[Info] Insert complete.\n";
    parseSQL(stdin);
    delete dbm;
    return 0;
}