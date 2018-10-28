#include "RID.h"

RID::RID(){
    this->pagenum = -1;
    this->slotnum = -1;
}

RID::RID(int p, int s){
    this->pagenum = p;
    this->slotnum = s;
}

bool RID::operator==(const RID &other){
    return ((other.pagenum == this->pagenum) && (other.slotnum == this->slotnum));
}