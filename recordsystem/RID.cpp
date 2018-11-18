#include "RID.h"

RID::RID(){
    this->pagenum = -1;
    this->slotnum = -1;
}

RID::RID(int raw){
    this->pagenum = raw >> 13;
    this->slotnum = raw & ((1 << 13)-1);
}

RID::RID(int p, int s){
    this->pagenum = p;
    this->slotnum = s;
}

int RID::toInt(){
    return ((this->pagenum << 13) | (this->slotnum));
}

bool RID::operator==(const RID &other){
    return ((other.pagenum == this->pagenum) && (other.slotnum == this->slotnum));
}