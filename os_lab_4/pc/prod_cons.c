#include "prod_cons.h"

struct sembuf Pprod[2] = {{BEMPTY, -1, SEM_UNDO}, {BINARY, -1, SEM_UNDO}};
struct sembuf Vprod[2] = {{BINARY, 1, SEM_UNDO}, {BFULL, 1, SEM_UNDO}};

struct sembuf Pcons[2] = {{BFULL, -1, SEM_UNDO}, {BINARY, -1, SEM_UNDO}};
struct sembuf Vcons[2] = {{BINARY, 1, SEM_UNDO}, {BEMPTY, 1, SEM_UNDO}};

void producer(const int semid, buf_t * const buf) {
    char sym_next;
    srand(time(NULL));
    while(1) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pprod, 2) == -1)
            errExit("semop");

        sym_next = 'a' + (char)((buf->sym_now - 'a' + 1) % 26);
        buf->buf[buf->prod_pos] = sym_next;
        buf->prod_pos = (buf->prod_pos + 1) % N_BUF;
        buf->sym_now = sym_next;
        printf("Производитель PID=%d положил '%c'", getpid(), sym_next);
        
        if (SHOWBUF) {
            printf("\t");
            for (size_t i = 0; i < N_BUF; ++i)
                printf("%c", buf->buf[i]);
            printf("\n");
        }

        if (semop(semid, Vprod, 2) == -1)
        errExit("semop");
    }
}

void consumer(const int semid, buf_t * const buf) {
    char symb;
    size_t tmp;
    srand(time(NULL));
    while(1) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pcons, 2) == -1)
            errExit("semop");

        tmp = buf->cons_pos;
        symb = buf->buf[buf->cons_pos];
        buf->cons_pos = (buf->cons_pos + 1) % N_BUF;
        printf("Потребитель   PID=%d взял    '%c'", getpid(), symb);
        if (SHOWBUF) {
            buf->buf[tmp] = '-';
            printf("\t");
            for (size_t i = 0; i < N_BUF; ++i)
                printf("%c", buf->buf[i]);
            printf("\n");
        }

        if (semop(semid, Vcons, 2) == -1)
            errExit("semop");
    }
}
