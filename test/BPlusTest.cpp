#include "../common.h"
#include "../indexsystem/BPlusTree.h"

// use after modifying MAXINDEXRECPERPAGE = 5 && MAXRECPEROVERFLOWPAGE = 2

void checkBPlus(){
    unlink("BPlusTest.TestCol.tree");
    unlink("BPlusTest.TestCol.key");

    string tableName = "BPlusTest";
    string colName = "TestCol";
    BPlusTree *bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    bt->insert(DataContainer::genIntData(5), 1);
    bt->insert(DataContainer::genIntData(5), 2);
    bt->insert(DataContainer::genIntData(5), 3);
    assert(bt->count(DataContainer::genIntData(5)) == 3);
    assert(bt->has(DataContainer::genIntData(5)));
    assert(bt->lesserCount(DataContainer::genIntData(5)) == 0);
    assert(bt->greaterCount(DataContainer::genIntData(5)) == 0);
    BPlusTreeIterator bit1 = bt->lowerBound(DataContainer::genIntData(5));
    bit1.setToBegin();
    assert(bit1.available());
    assert(bit1.getValue() == 1);
    bit1.next();
    assert(bit1.getValue() == 2);
    bit1.next();
    assert(bit1.getValue() == 3);
    bit1.next();
    assert(!bit1.available());
    bit1.setToBegin();
    bit1.next();
    bit1.next();
    bit1.getValue();
    bit1.previous();
    assert(bit1.getValue() == 2);
    bit1.previous();
    assert(bit1.getValue() == 1);
    bit1.previous();
    assert(!bit1.available());
    cout << "b plus insert 1 complete!\n";
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    bt->insert(DataContainer::genIntData(3), 4);
    bt->insert(DataContainer::genIntData(10), 5);
    bt->insert(DataContainer::genIntData(15), 6);
    assert(bt->has(DataContainer::genIntData(15)));
    assert(bt->has(DataContainer::genIntData(3)));
    assert(!bt->has(DataContainer::genIntData(4)));
    assert(bt->count(DataContainer::genIntData(5)) == 3);
    assert(bt->lesserCount(DataContainer::genIntData(5)) == 1);
    assert(bt->greaterCount(DataContainer::genIntData(5)) == 2);
    assert(bt->totalCount() == 6);
    cout << "b plus insert 2 complete!\n";

    

    unlink("BPlusTest.TestCol.tree");
    unlink("BPlusTest.TestCol.key");
 
}

int main(){
    srand((unsigned int)time(NULL));
    checkBPlus();
    return 0;
}