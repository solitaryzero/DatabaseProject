/*
 * parser.h
 *
 *  Created on: 2014年12月7日
 *      Author: lql
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "pagedef.h"
#include <cstring>
using namespace std;
#define LL_TYPE 0
#define DB_TYPE 1
#define ST_TYPE 2
#define L 0
#define G 3
#define LE 1
#define GE 2
#define E 4
#define IS 5
#define UNI 0
#define UNUNI 1
#define N 0
#define UN 1
#define ALL 0
#define RANGE 1
#define ISNULL 2
#define NOTHING 3
extern int tmp;
template<class Key>
extern int keyu(uchar* a, uchar* b);
template<class Key>
extern int keyn(uchar* a, uchar* b);
extern int su(uchar* a, uchar* b);
extern int sn(uchar* a, uchar* b);
extern cf* kcmp[3][2];
#endif /* PARSER_H_ */
