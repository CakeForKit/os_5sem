#ifndef PROD_CONS_H__
#define PROD_CONS_H__

#include "consts.h"

void producer(const int semid, buf_t * const buf);
void consumer(const int semid, buf_t * const buf);

#endif // PROD_CONS_H__