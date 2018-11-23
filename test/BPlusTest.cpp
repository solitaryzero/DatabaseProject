#include "../common.h"
#include "../indexsystem/BPlusTree.h"

// please run after modifying MAXINDEXRECPERPAGE = 5 && MAXRECPEROVERFLOWPAGE = 2

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
    cout << "b plus insert 1(duplicate keys) complete!\n";
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
    cout << "b plus insert 2(unique keys) complete!\n";
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    bt->insert(DataContainer::genIntData(1), 7);
    assert(bt->treeFile->header->pageCount == 3);
    bt->insert(DataContainer::genIntData(31), 8);
    assert(bt->treeFile->header->pageCount == 5);
    assert(bt->treeFile->header->firstLeaf == 1);
    assert(bt->treeFile->header->lastLeaf == 5);
    assert(bt->treeFile->header->rootPageId == 4);
    BPlusTreeIterator bit3(bt);
    bit3.setToBegin();
    assert(bit3.getValue() == 7);
    bit3.next();
    bit3.next();
    assert(bit3.getValue() == 1);
    bit3.next();
    bit3.next();
    bit3.next();
    bit3.next();
    bit3.next();
    assert(bit3.getValue() == 8);
    bit3.next();
    assert(!bit3.available());
    bit3.setToBegin();
    bit3.next();
    bit3.next();
    bit3.next();
    bit3.previousKey();
    assert(bit3.getValue() == 4);
    assert(bt->count(DataContainer::genIntData(5)) == 3);
    assert(bt->lesserCount(DataContainer::genIntData(5)) == 2);
    assert(bt->greaterCount(DataContainer::genIntData(5)) == 3);
    cout << "b plus insert 3(split leaf) complete!\n";
    bt->deleteIndex();
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    for (int i=0;i<15;i++){
        bt->insert(DataContainer::genIntData(i), i);
    }
    assert(bt->totalCount() == 15);
    for (int i=0;i<15;i++){
        assert(bt->count(DataContainer::genIntData(i)) == 1);
        vector<RID> res = bt->getRIDs(DataContainer::genIntData(i));
        assert(res.size() == 1);
        assert(res[0].toInt() == i);
    }

    for (int i=0;i<15;i++){
        assert(bt->count(DataContainer::genIntData(i)) == 1);
        assert(bt->lesserCount(DataContainer::genIntData(i)) == i);
        assert(bt->greaterCount(DataContainer::genIntData(i)) == 14-i);
    }

    for (int i=0;i<15;i+=2){
        bt->insert(DataContainer::genIntData(i), i+15);
    }

    BPlusTreeIterator bit4 = bt->lowerBound(DataContainer::genIntData(4));
    assert(bit4.getValue() == 4);
    bit4 = bt->upperBound(DataContainer::genIntData(4));
    assert(bit4.getValue() == 5);

    assert(bt->totalCount() == 23);
    for (int i=0;i<15;i++){
        assert(bt->count(DataContainer::genIntData(i)) == (2-(i%2)));
        vector<RID> res = bt->getRIDs(DataContainer::genIntData(i));
        assert(res.size() == (unsigned int)(2-(i%2)));
        assert(res[0].toInt() == i);
    }
    cout << "b plus insert 4(split intermediate) complete!\n";
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    BPlusTreeIterator bit5(bt);
    //正向逐个value遍历
    bit5.setToBegin();
    for (int i=0;i<15;i++){
        assert(bit5.getValue() == i);
        bit5.next();
        if (i%2 == 0){
            assert(bit5.getValue() == 15+i);
            bit5.next();
        }
    }
    assert(!bit5.available());
    //正向逐个key遍历
    bit5.setToBegin();
    for (int i=0;i<15;i++){
        assert(bit5.getValue() == i);
        bit5.nextKey();
    }
    assert(!bit5.available());
    //反向逐个value遍历
    bit5.setToBegin();
    for (int i=0;i<15-1;i++){
        bit5.nextKey();
    }
    bit5.next();
    assert(bit5.available());
    for (int i=14;i>=0;i--){
        if (i%2 == 0){
            assert(bit5.getValue() == 15+i);
            bit5.previous();
        }
        assert(bit5.getValue() == i);
        bit5.previous();
    }
    assert(!bit5.available());
    //反向逐个key遍历
    bit5.setToBegin();
    for (int i=0;i<15-1;i++){
        bit5.nextKey();
    }
    assert(bit5.available());
    for (int i=14;i>=0;i--){
        assert(bit5.getValue() == i);
        bit5.previousKey();
    }
    assert(!bit5.available());

    cout << "b plus insert iteration complete!\n";
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    for (int i=0;i<15;i+=2){
        bt->remove(DataContainer::genIntData(i), i+15);
    }
    assert(bt->totalCount() == 15);
    BPlusTreeIterator bit6(bt);
    bit6.setToBegin();
    for (int i=0;i<15;i++){
        assert(bit6.getValue() == i);
        bit6.next();
    }
    assert(!bit6.available());
    for (int i=0;i<15;i++){
        assert(bt->count(DataContainer::genIntData(i)) == 1);
        assert(bt->lesserCount(DataContainer::genIntData(i)) == i);
        assert(bt->greaterCount(DataContainer::genIntData(i)) == 14-i);
        assert(bt->getRIDs(DataContainer::genIntData(i))[0].toInt() == i);
    }

    cout << "b plus insert delete 1(duplicate key) complete!\n";
    delete(bt);

    bt = new BPlusTree(tableName, colName, varTypes::INT_TYPE);
    for (int i=0;i<15;i++){
        assert(bt->count(DataContainer::genIntData(i)) == 1);
        assert(bt->lesserCount(DataContainer::genIntData(i)) == i);
        assert(bt->greaterCount(DataContainer::genIntData(i)) == 14-i);
        assert(bt->getRIDs(DataContainer::genIntData(i))[0].toInt() == i);
    }
    
    for (int i=1;i<15;i+=2){
        bt->remove(DataContainer::genIntData(i), i);
    }
    
    assert(bt->totalCount() == 8);
    BPlusTreeIterator bit7(bt);
    bit7.setToBegin();
    for (int i=0;i<8;i++){
        assert(bit7.getValue() == i*2);
        bit7.next();
    }
    assert(!bit7.available());
    for (int i=0;i<8;i++){
        assert(bt->count(DataContainer::genIntData(i*2)) == 1);
        assert(bt->lesserCount(DataContainer::genIntData(i*2)) == i);
        assert(bt->greaterCount(DataContainer::genIntData(i*2)) == 7-i);
    }

    bit7 = bt->lowerBound(DataContainer::genIntData(3));
    assert(bit7.available());
    assert(bit7.getValue() == 2);
    bit7 = bt->upperBound(DataContainer::genIntData(3));
    assert(bit7.getValue() == 4);
    assert(bt->getRIDs(DataContainer::genIntData(3)).size() == 0);

    cout << "b plus insert delete 2(unique key & merge) complete!\n";
    delete(bt);

    unlink("BPlusTest.TestCol.tree");
    unlink("BPlusTest.TestCol.key");
}

int main(){
    srand((unsigned int)time(NULL));
    checkBPlus();
    return 0;
}