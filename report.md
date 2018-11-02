# 记录管理模块
记录管理模块部分代码在`./recordsystem/`目录下．    
该模块主要分为两部分：     
1. 数据文件的生成，插入，删除与查询．相关文件为    
```
AbstractRecordFile.h
FixedRecordFile.h/cpp
UnfixedRecordFile.h/cpp
RID.h/cpp
```
`AbstractRecordFile`为数据文件的抽象类，提供了增删改查与遍历记录的函数接口．具体实现分定长记录文件(`FixedRecordFile`)与非定长记录文件(`UnfixedRecordFile`)两种．    
定长记录文件使用第二章中提到的bitmap记录每个页中哪些位置可用，非定长记录文件则在每页末尾记录每个slot的起始位置与长度．如果记录类型中有任意个数的非定长域，则会采用非定长记录文件．    
此部分单元测试文件见`RecordTest.cpp`．    

2. 数据表的维护，数据在字节流与可读内容间的转化．相关文件为    
```
ColumnInfo.h/cpp
DataOperands.h/cpp
RecordConverter.h/cpp
TableInfo.h/cpp
RecordManager.h/cpp
```
`ColumnInfo`记录了某一个具体列的信息，如类型，列名，大小等;   
`DataOperands`为数据类别提供了相关操作，如名字与对应enum间的转化，两个数据类型间的比较与相加等等;   
`RecordConverter`可以在给定表信息的情况下，将数据文件中的原始字节流与结构化的数据间互相进行转化，并支持修改与读取某一个具体域上的数据;    
`TableInfo`维护了某个具体表的信息，并可以通过它来对数据文件进行修改;    
`RecordManager`是以上功能的综合，可以进行表的创建,打开与删除．    
此部分单元测试文件见`TableTest.cpp`．    

为了维护内存的方便，所有数据都以`std::vector<unsigned char>`的格式储存，并使用`shared_ptr`传递，防止内存泄漏;在需要转化的情况下，才会通过`RecordConverter`中相关函数转化成对应类型.    
每一条record的字节流储存格式与第二章课件中的格式相同(标志位-定长数据-null位图-非定长数据).   