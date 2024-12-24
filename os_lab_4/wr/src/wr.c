#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define max_time_sleep 1

#define writers_cnt     3   // 3
#define readers_cnt     6   // 5

#define active_writer       0
#define numb_of_readers  1
#define writers_queue    2
#define readers_queue   3
#define bin_sem 4

struct sembuf start_read[] = {
    {readers_queue, 1, 0},
    {active_writer, 0, 0},
    {writers_queue, 0, 0},
    {numb_of_readers, 1, 0},
    {readers_queue, -1, 0}
};
struct sembuf stop_read[] = { {numb_of_readers, -1, 0} };

struct sembuf start_write[] = {
    {writers_queue, 1, 0},
    {numb_of_readers, 0, 0},
    {bin_sem, -1, 0},
    {active_writer, 1, 0},
    {writers_queue, -1, 0}
};
struct sembuf stop_write[] = { {active_writer, -1, 0}, {bin_sem, 1, 0} };

int f_sigint = 1;

void handler(int sig_numb) {
    printf("PID=%d signal=%d\n", getpid(),sig_numb);
    f_sigint = 0;
}

void reader(int semid, char *buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (max_time_sleep + 1));

        if (semop(semid, start_read, 5) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop(start_read) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            perror(err_msg); 
            exit(EXIT_FAILURE);
        }
        printf("Читатель PID=%d считал '%c'\n", getpid(), *buf);

        if (semop(semid, stop_read, 1) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            perror(err_msg); 
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

void writer(int semid, char *buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (max_time_sleep + 1));
        
        if (semop(semid, start_write, 5) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop(start_write) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            perror(err_msg); 
            exit(EXIT_FAILURE);
        }
        if (*buf == 'z')
            *buf = 'a';
        else
            (*buf)++;
        // sleep(1);
        printf("Писатель PID=%d записал '%c'\n", getpid(), *buf);

        if (semop(semid, stop_write, 2) == -1){
            perror("semop"); 
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int semid, shmid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *buf;
    int cpid, wstatus;
    pid_t w;
    key_t key;

    if (signal(SIGINT, handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }

    key = ftok(argv[0], 2);
    if (key == -1){
        perror("ftok"); 
        exit(EXIT_FAILURE);
    }

    shmid = shmget(key, 1, IPC_CREAT | perms);
    if (shmid == -1) {
        perror("shmid"); 
        exit(EXIT_FAILURE);
    }
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1){
        perror("shmat"); 
        exit(EXIT_FAILURE);
    }
    *buf = 'a';

    semid = semget(key, 5, IPC_CREAT | perms);
    if (semid == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, active_writer, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, numb_of_readers, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, writers_queue, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, readers_queue, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, bin_sem, SETVAL, 1) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < writers_cnt; ++i) {
        if ((cpid = fork()) == -1){
            perror("fork"); 
            exit(EXIT_FAILURE);
        }
        else if (cpid == 0)
            writer(semid, buf);
    }
    for (size_t i = 0; i < readers_cnt; ++i) {
        if ((cpid = fork()) == -1) {
            perror("ftok"); 
            exit(EXIT_FAILURE);
        }
        else if (cpid == 0)
            reader(semid, buf);
    }

    for (size_t i = 0; i < writers_cnt + readers_cnt; ++i) {
        w = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED); // until one of its children terminates (ANY child process.)
        if (w == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: waitpid PID=%d, errno=%d", getpid(), errno);
            perror(err_msg); 
            exit(EXIT_FAILURE);
        }

        printf("PID=%d ", w);
        if (WIFEXITED(wstatus)) {                                    
            printf("exited, status = %d ", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d ", WTERMSIG(wstatus));
        } else if (WIFSTOPPED(wstatus)) {
            printf("stopped by signal %d ", WSTOPSIG(wstatus));
        } else if (WIFCONTINUED(wstatus)) {
            printf("continued ");
        }
        printf("\n");
    }

    if (shmdt(buf) == -1) {
        perror("shmdt"); 
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("shmctl"); 
        exit(EXIT_FAILURE);
    }
    
    return 0;
}