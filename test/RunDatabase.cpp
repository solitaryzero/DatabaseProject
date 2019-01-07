#include "common.h"
#include "./databasesystem/Glue.h"

int main(int argc,char* argv[]){
    dbm = new DatabaseManager();
    if (argc == 1){
        parseSQL(stdin);
    } else if (argc >= 2){
        /*
        freopen(argv[1], "r", stdin);
        if (argc >= 3){
            freopen(argv[2], "w", stdout);
        }
        parseSQL(stdin);
        fclose(stdin);
        fclose(stdout);
        */
        parseSQL("../dataset/create.sql");
        parseSQL("../dataset/restaurant.sql");
        parseSQL("../dataset/customer.sql");
        parseSQL("../dataset/food.sql");
        parseSQL("../dataset/orders.sql");
        //parseSQL(stdin);
    }
    delete dbm;
    return 0;
}