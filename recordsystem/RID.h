#ifndef RID_H
#define RID_H

class RID{
public:
    RID();
    RID(int p, int s);
    int pagenum;
    int slotnum;

    bool operator==(const RID &other);
};

#endif