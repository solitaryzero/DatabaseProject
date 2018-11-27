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

# 索引管理模块
索引管理模块部分代码在`./indexsystem/`目录下．      
索引集中使用B+树进行管理．为了方便管理，对原始B+树进行了以下修改：     
1. 由于RID的slotNum不会超过2^13，而一般pageNum也不会过大，因此为RID类添加toInt()函数和从单个int的构造函数，允许其与int互相转化．    
2. 由于实际插入的key值长度不固定，额外建立一个文件储存各个key值，这样就可以在树中只存储每个key在键值文件中的RID．    
3. 树中每个单元存储三个值：(key的位置, value(RID对应的int)，计数)．计数在中间节点代表该节点下有多少个元素，在叶节点代表该key对应多少rid．     
4. 处理重复的key值时，没有使用课上讲的增加新column方法，而是使用了溢出页：当叶节点某一个位置的count大于1，对应的value不是某个RID值，而是溢出页编号．溢出页中只存储int形式的RID.    
5. 在进行插入和删除操作时，在向下进入下一个节点前就会判断其大小是否合法，提前进行拆分/旋转/合并操作．这样虽然可能导致额外开销（如交叉进行插入与删除操作），但是由于树总高度不会增加，所以复杂度不变，只是部分增大常数;这样可以避免向父节点插入数据带来的麻烦．    
6. 叶节点中，每个单元都储存具体值;中间节点中，第一个单元对应的key位置不存在，对应小于该节点中所有key的指针;也就是说，除了每一层中最左侧的中间节点，该单元无意义．    
7. 页节点组成一整个双向链表，可以通过BPlusTreeIterator进行前后移动，完成遍历．    

B+树的接口如下：
```
BPlusTree(string tableName, string colName, varTypes type);     //根据表名，列名，列数据类型创建B+树
~BPlusTree();                                                   //析构函数

void insert(data_ptr key, int rid);                             //插入一条键值为key的记录，其位置为rid
void remove(data_ptr key, int rid);                             //删除一条键值为key，位置为rid的记录
bool has(data_ptr key);                                         //该列中是否存在值为key的记录
int count(data_ptr key);                                        //该列中值为key的记录数量
int lesserCount(data_ptr key);                                  //该列中值小于key的记录数量
int greaterCount(data_ptr key);                                 //该列中值大于key的记录数量
BPlusTreeIterator lowerBound(data_ptr key);                     //第一个=key或最后一个<key的位置
BPlusTreeIterator upperBound(data_ptr key);                     //第一个>key的位置
vector<RID> getRIDs(data_ptr key);                              //获取所有键值为key的RID
int totalCount();                                               //B+树中总记录数量
```

叶节点结构：
```
键位置[0]               键位置[1]               ...       键位置[n-1]
溢出页ID或对应RID[0]     溢出页ID或对应RID[1]     ...       溢出页ID或对应RID[n-1]
对应记录数量[0]          对应记录数量[1]          ...       对应记录数量[n-1]
```
中间节点结构：
```
(无意义)                键位置[1]               ...       键位置[n-1]
溢出页ID或对应RID[0]     溢出页ID或对应RID[1]     ...       溢出页ID或对应RID[n-1]
对应记录数量[0]          对应记录数量[1]          ...       对应记录数量[n-1]
```