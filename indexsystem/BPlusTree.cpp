#include "BPlusTree.h"

BPlusTree::BPlusTree(string tableName, string colName){
    this->tableName = tableName;
    this->colName = colName;
}

void BPlusTree::insert(data_ptr key, int rid){

}

void BPlusTree::remove(data_ptr key, int rid){
    
}

bool BPlusTree::has(data_ptr key){
    return false;
}

int BPlusTree::count(data_ptr key){
    return 0;
}

int BPlusTree::greaterCount(data_ptr key){
    return 0;
}

BPlusTreeIterator::BPlusTreeIterator(BPlusTree* bt){
    this->tree = bt;
    this->currentNode = nullptr;
    this->currentKeyPos = -1;
    this->currentValuePos = -1;
}

BPlusTreeIterator::BPlusTreeIterator(BPlusTree* bt, BPlusNode* bn, int keyn, int valn){
    this->tree = bt;
    this->currentNode = bn;
    this->currentKeyPos = keyn;
    this->currentValuePos = valn;
}

data_ptr BPlusTreeIterator::getKey(){
    return nullptr;
}

int BPlusTreeIterator::getValue(){
    return 0;
}

void BPlusTreeIterator::next(){

}

void BPlusTreeIterator::nextKey(){
    
}

void BPlusTreeIterator::previous(){
    
}

void BPlusTreeIterator::previousKey(){
    
}

bool BPlusTreeIterator::hasNext(){
    return false;
}

void BPlusTreeIterator::setToBegin(){
    
}