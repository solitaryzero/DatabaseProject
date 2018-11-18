#ifndef RID_H
#define RID_H

class RID{
public:
    RID();
    RID(int raw);
    RID(int p, int s);
    int toInt();
    int pagenum;
    int slotnum;

    bool operator==(const RID &other);
};

#endif