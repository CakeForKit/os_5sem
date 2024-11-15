/* svshm_string.h

    Licensed under GNU General Public License v2 or later.
*/
#ifndef SVSHM_STRING_H
#define SVSHM_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds     *buf;
    unsigned short      *array;
#if defined(__linux__)
    struct seminfo      *__buf;
#endif
};

#define MEM_SIZE 4096

#endif  // include guard