#include "utils/compare.h"

int tmp = 0;
template<class Key>
int keyu(uchar* a, uchar* b) {
	Key c, d;
	memcpy(&c, a, sizeof(Key));
	memcpy(&d, b, sizeof(Key));
	return ((c < d) ? -1 : ((c > d) ? 1 : 0));
}
template<class Key>
int keyn(uchar* a, uchar* b) {
    int res = keyu<Key>(a, b);
    return (res != 0) ? res : keyu<ll>(a + sizeof(Key), b + sizeof(Key));
}
int su(uchar* a, uchar* b) {
	int res = strcmp((char*)a, (char*)b);
	return (res > 0) ? 1 : ((res < 0) ? -1 : 0);
}
int sn(uchar* a, uchar* b) {
	int res = strcmp((char*)a, (char*)b);
	res = (res > 0) ? 1 : ((res < 0) ? -1 : 0);
	return (res != 0) ? res : keyu<ll>(a + tmp, b + tmp);
}
cf* kcmp[3][2] = {
	{&keyu<ll>, &keyn<ll>},
	{&keyu<db>, &keyn<db>},
	{&su, &sn}
};