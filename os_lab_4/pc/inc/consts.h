#ifndef CONST_H
#define CONST_H

#define DEBUG 1

#define BINARY 0
#define BEMPTY 1
#define BFULL 2

#define PROD_CNT 4
#define CONS_CNT 5

#define N_BUF 10

#define MAX_TIME_SLEEP 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

struct buf_struct {
    size_t prod_pos;
    size_t cons_pos;
    char sym_now;
    char buf[N_BUF];
};
typedef struct buf_struct buf_t;

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#endif // CONST_H