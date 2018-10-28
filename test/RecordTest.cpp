#include "../common.h"
#include "../recordsystem/FixedRecordFile.h"
#include "../recordsystem/UnfixedRecordFile.h"

void checkUnfixed(){
    system("rm -f UnfixedRec.test");
    srand((unsigned int)time(NULL));
    UnfixedRecordFile *uf = new UnfixedRecordFile("UnfixedRec.test");
    data_ptr t[5];
    t[0] = DataContainer::genDataContainer(20);
    t[1] = DataContainer::genDataContainer(20);
    t[2] = DataContainer::genDataContainer(2000);
    t[3] = DataContainer::genDataContainer(4000);
    t[4] = DataContainer::genDataContainer(4000);
    for (int i=0;i<5;i++){
        for (size_t j=0;j<t[i]->size();j++){
            t[i]->data()[j] = rand()%256;
        }
    }    

    for (int i=0;i<5;i++){
        uf->insertData(t[i]);
    }

    uf->deleteData(RID(1, 2));

    uf->closeFile();
    delete(uf);
    uf = new UnfixedRecordFile("UnfixedRec.test");

    data_ptr r0 = uf->getData(RID(1, 0));
    assert(t[0]->size() == r0->size());
    assert(memcmp(t[0]->data(), r0->data(), t[0]->size()) == 0);
    data_ptr r1 = uf->getData(RID(1, 1));
    assert(t[1]->size() == r1->size());
    assert(memcmp(t[1]->data(), r1->data(), t[1]->size()) == 0);

    data_ptr r2 = uf->getData(RID(1, 2));
    assert(r2 == nullptr);

    data_ptr r4 = uf->getData(RID(2, 0));
    assert(t[4]->size() == r4->size());
    assert(memcmp(t[4]->data(), r4->data(), t[4]->size()) == 0);

    printf("Unfixed test crud complete!\n");

    uf->closeFile();
    delete(uf);
    uf = new UnfixedRecordFile("UnfixedRec.test");

    data_ptr it0 = uf->firstData();
    assert(it0 != nullptr);
    assert(t[0]->size() == it0->size());
    assert(memcmp(t[0]->data(), it0->data(), t[0]->size()) == 0);

    data_ptr it1 = uf->nextData();
    assert(t[1]->size() == it1->size());
    assert(memcmp(t[1]->data(), it1->data(), t[1]->size()) == 0);

    data_ptr it3 = uf->nextData();
    assert(t[3]->size() == it3->size());
    assert(memcmp(t[3]->data(), it3->data(), t[3]->size()) == 0);

    data_ptr it4 = uf->nextData();
    assert(t[4]->size() == it4->size());
    assert(memcmp(t[4]->data(), it4->data(), t[4]->size()) == 0);

    data_ptr it5 = uf->nextData();
    assert(it5 == nullptr);

    data_ptr it6 = uf->nextData();
    assert(it6 == nullptr);

    printf("Unfixed test iteration complete!\n");
}

int main(){
    checkUnfixed();
    return 0;
}