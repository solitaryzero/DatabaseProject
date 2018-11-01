#include "../recordsystem/RecordManager.h"

void checkUnfixed(){
    //system("rm -f TestTable.tbinfo");
    //system("rm -f TestTable.tbdata");
    RecordManager *rm = new RecordManager();

    rm->newTable("TestTable");
    rm->tinfo->addNewColumn("IntCol", varTypes::INT_TYPE, DataOperands::getTypeSize(varTypes::INT_TYPE));
    rm->tinfo->addNewColumn("FloatCol", varTypes::FLOAT_TYPE, DataOperands::getTypeSize(varTypes::FLOAT_TYPE));
    rm->tinfo->addNewColumn("CharCol", varTypes::CHAR_TYPE, 20);
    rm->tinfo->addNewColumn("VarcharCol1", varTypes::VARCHAR_TYPE, 30);
    rm->tinfo->addNewColumn("VarcharCol2", varTypes::VARCHAR_TYPE, 40);
    rm->tinfo->showTableInfo();

    cout << "=============\n";

    rm->openDataFile();
    assert(rm->tinfo->cvt != nullptr);
    map<int, data_ptr> vmap;
    vmap.insert({0, DataContainer::genIntData(1)});
    vmap.insert({1, DataContainer::genFloatData(2.3)});
    vmap.insert({2, DataContainer::genStringData("Hello")});
    vmap.insert({3, DataContainer::genStringData(" world!")});
    vmap.insert({4, nullptr});
    rm->tinfo->cvt->fromIndexValueMap(vmap);
    rm->tinfo->cvt->showValues();

    assert(rm->unfixedFile != nullptr);
    assert(rm->fixedFile == nullptr);
    rm->unfixedFile->insertData(rm->tinfo->cvt->toByteArray());
    delete(rm);

    cout << "=============\n";

    rm = new RecordManager();
    rm->openTable("TestTable");
    assert(rm->unfixedFile != nullptr);
    assert(rm->fixedFile == nullptr);
    data_ptr res = rm->unfixedFile->getData(RID(1,0));
    rm->tinfo->cvt->fromByteArray(res);
    rm->tinfo->cvt->showValues();
    delete(rm);

    cout << "=============\n";

    rm = new RecordManager();
    rm->dropTable("TestTable");
    delete(rm);

    cout << "=============\n";
    cout << "table test(unfixed) completed!\n\n\n";
}

void checkFixed(){
    RecordManager *rm = new RecordManager();

    rm->newTable("TestTable2");
    rm->tinfo->addNewColumn("IntCol", varTypes::INT_TYPE, DataOperands::getTypeSize(varTypes::INT_TYPE));
    rm->tinfo->addNewColumn("FloatCol", varTypes::FLOAT_TYPE, DataOperands::getTypeSize(varTypes::FLOAT_TYPE));
    rm->tinfo->addNewColumn("CharCol", varTypes::CHAR_TYPE, 20);
    rm->tinfo->showTableInfo();

    cout << "=============\n";

    rm->openDataFile();
    assert(rm->tinfo->cvt != nullptr);
    map<string, data_ptr> vmap;
    vmap.insert({"CharCol", nullptr});
    vmap.insert({"FloatCol", DataContainer::genFloatData(2.3)});
    vmap.insert({"IntCol", DataContainer::genIntData(1)});
    
    rm->tinfo->cvt->fromNameValueMap(vmap);
    rm->tinfo->cvt->showValues();

    assert(rm->fixedFile != nullptr);
    assert(rm->unfixedFile == nullptr);

    
    rm->fixedFile->insertData(rm->tinfo->cvt->toByteArray());

    cout << "=============\n";

    vmap.clear();
    vmap.insert({"CharCol", DataContainer::genStringData("null")});
    vmap.insert({"FloatCol", nullptr});
    vmap.insert({"IntCol", DataContainer::genIntData(114514)});
    
    rm->tinfo->cvt->fromNameValueMap(vmap);
    rm->tinfo->cvt->showValues();

    assert(rm->fixedFile != nullptr);
    assert(rm->unfixedFile == nullptr);
    rm->fixedFile->insertData(rm->tinfo->cvt->toByteArray());
    delete(rm);

    cout << "=============\n";

    rm = new RecordManager();
    rm->openTable("TestTable2");
    assert(rm->fixedFile != nullptr);
    assert(rm->unfixedFile == nullptr);
    data_ptr res = rm->fixedFile->getData(RID(1,1));
    rm->tinfo->cvt->fromByteArray(res);
    rm->tinfo->cvt->showValues();
    delete(rm);

    cout << "=============\n";

    rm = new RecordManager();
    rm->dropTable("TestTable2");
    delete(rm);

    cout << "=============\n";
    cout << "table test(fixed) completed!\n";
}

int main(){
    checkUnfixed();
    checkFixed();
    return 0;
}