#include "../recordsystem/RecordManager.h"

void checkUnfixed(){
    //system("rm -f TestTable.tbinfo");
    //system("rm -f TestTable.tbdata");

    RecordManager *rm = new RecordManager();
    shared_ptr<TableInfo> tinfo;
    tinfo = rm->newTable("TestTable");
    tinfo->addNewColumn("IntCol", varTypes::INT_TYPE, DataOperands::getTypeSize(varTypes::INT_TYPE));
    tinfo->addNewColumn("FloatCol", varTypes::FLOAT_TYPE, DataOperands::getTypeSize(varTypes::FLOAT_TYPE));
    tinfo->addNewColumn("CharCol", varTypes::CHAR_TYPE, 20);
    tinfo->addNewColumn("VarcharCol1", varTypes::VARCHAR_TYPE, 30);
    tinfo->addNewColumn("VarcharCol2", varTypes::VARCHAR_TYPE, 40);
    tinfo->showTableInfo();

    cout << "=============\n";

    rm->openDataFile(tinfo);
    assert(tinfo->cvt != nullptr);
    map<int, data_ptr> vmap;
    vmap.insert({0, DataContainer::genIntData(1)});
    vmap.insert({1, DataContainer::genFloatData(2.3)});
    vmap.insert({2, DataContainer::genStringData("Hello")});
    vmap.insert({3, DataContainer::genStringData(" world!")});
    vmap.insert({4, nullptr});
    tinfo->cvt->fromIndexValueMap(vmap);
    tinfo->cvt->showValues();

    tinfo->dataFile->insertData(tinfo->cvt->toByteArray());

    cout << "=============\n";

    tinfo = rm->openTable("TestTable");
    data_ptr res = tinfo->dataFile->getData(RID(1,0));
    tinfo->cvt->fromByteArray(res);
    tinfo->cvt->showValues();

    cout << "=============\n";

    rm->dropTable("TestTable");
    cout << "Dropped TestTable\n";

    cout << "=============\n";
    cout << "table test(unfixed) completed!\n\n\n";
    delete(rm);
}

void checkFixed(){
    RecordManager *rm = new RecordManager();
    shared_ptr<TableInfo> tinfo;
    tinfo = rm->newTable("TestTable2");
    tinfo->addNewColumn("IntCol", varTypes::INT_TYPE, DataOperands::getTypeSize(varTypes::INT_TYPE));
    tinfo->addNewColumn("FloatCol", varTypes::FLOAT_TYPE, DataOperands::getTypeSize(varTypes::FLOAT_TYPE));
    tinfo->addNewColumn("CharCol", varTypes::CHAR_TYPE, 20);
    tinfo->showTableInfo();

    cout << "=============\n";

    rm->openDataFile(tinfo);
    assert(tinfo->cvt != nullptr);
    map<string, data_ptr> vmap;
    vmap.insert({"CharCol", nullptr});
    vmap.insert({"FloatCol", DataContainer::genFloatData(2.3)});
    vmap.insert({"IntCol", DataContainer::genIntData(1)});
    
    tinfo->cvt->fromNameValueMap(vmap);
    tinfo->cvt->showValues();
    
    tinfo->dataFile->insertData(tinfo->cvt->toByteArray());

    cout << "=============\n";

    vmap.clear();
    vmap.insert({"CharCol", DataContainer::genStringData("null")});
    vmap.insert({"FloatCol", nullptr});
    vmap.insert({"IntCol", DataContainer::genIntData(114514)});
    
    tinfo->cvt->fromNameValueMap(vmap);
    tinfo->cvt->showValues();

    tinfo->dataFile->insertData(tinfo->cvt->toByteArray());
    
    cout << "=============\n";

    tinfo = rm->openTable("TestTable2");
    data_ptr res = tinfo->dataFile->getData(RID(1,1));
    tinfo->cvt->fromByteArray(res);
    tinfo->cvt->showValues();

    cout << "=============\n";

    rm->dropTable("TestTable2");
    cout << "Dropped TestTable2\n";

    cout << "=============\n";
    cout << "table test(fixed) completed!\n";
    delete(rm);
}

int main(){
    checkUnfixed();
    checkFixed();
    return 0;
}